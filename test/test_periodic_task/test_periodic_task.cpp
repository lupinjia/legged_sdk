#include "common/periodic_task.hpp"

#include <random>
#include <vector>
#include <cmath>
#include <memory>
#include <sstream>
#include <sched.h>
#include <stdbool.h>

using namespace legged::common;

// Create a stress-testing function generator. This returns a lambda that performs
// mixed stress work each invocation: short busy-wait CPU work, occasional memory
// churn, occasional blocking sleep longer than the period (to force missed
// deadlines), and yields to encourage context switches.
// Per-task execution statistics (thread-safe)
struct ExecStats {
    std::atomic<long long> count{0};        // number of executions
    std::atomic<long long> sum_us{0};       // total runtime in microseconds
    std::atomic<long long> max_us{0};       // max runtime in microseconds
    // period (time between invocation starts) statistics
    std::atomic<long long> count_period{0};
    std::atomic<long long> sum_period_us{0};
    std::atomic<long long> max_period_us{0};
    // last call time in microseconds since steady_clock epoch (0 = not set)
    std::atomic<long long> last_call_us{0};
};

bool first_run[4] = {true, true, true, true};

static std::function<void()> makeStressFunction(int id, float period_us, std::shared_ptr<ExecStats> stats)
{
    return [id, period_us, stats]() {

        static thread_local std::mt19937 rng((unsigned)std::hash<std::thread::id>{}(std::this_thread::get_id()) ^ (unsigned)std::chrono::steady_clock::now().time_since_epoch().count());
        std::uniform_real_distribution<float> probDist(0.0f, 1.0f);
        float p = probDist(rng);
        // Measure inter-call period: record current start time in us and compute delta
        auto now_tp = std::chrono::steady_clock::now();
        long long now_us = std::chrono::duration_cast<std::chrono::microseconds>(now_tp.time_since_epoch()).count();
        long long prev_us = stats ? stats->last_call_us.exchange(now_us, std::memory_order_relaxed) : 0;
        if (prev_us != 0 && stats) {
            long long delta = now_us - prev_us;
            stats->count_period.fetch_add(1, std::memory_order_relaxed);
            stats->sum_period_us.fetch_add(delta, std::memory_order_relaxed);
            long long prev_max_p = stats->max_period_us.load(std::memory_order_relaxed);
            while (delta > prev_max_p && !stats->max_period_us.compare_exchange_weak(prev_max_p, delta, std::memory_order_relaxed)) {
                // retry
            }
        }

        // Measure execution time of this invocation
        auto start = std::chrono::steady_clock::now();
        // For a 1ms (1000us) period we want mostly short work (<70% of period),
        // occasional light memory touch, and rare overloads that exceed the period
        if (p < 0.60f) {
            // short CPU-bound busy work: between 5% and 60% of the period
            float frac = std::uniform_real_distribution<float>(0.05f, 0.60f)(rng);
            int target_us = std::max<int>(1, static_cast<int>(period_us * frac));
            auto t0 = std::chrono::high_resolution_clock::now();
            while (std::chrono::duration<float, std::micro>(std::chrono::high_resolution_clock::now() - t0).count() < target_us) {
                // lightweight math to keep CPU busy but fast per-iteration
                volatile double x = 0.0;
                for (int i = 0; i < 20; ++i) x += std::sin(i) * std::sqrt(i + 1);
            }
        } else {
            // small memory churn: allocate a modest buffer and touch some pages
            // sizes tuned for low overhead on 1ms tasks
            std::uniform_int_distribution<size_t> sizeDist(128, 2048); // ints => up to ~16KB
            size_t size = sizeDist(rng);
            std::vector<int> v;
            v.reserve(size);
            for (size_t i = 0; i < size; ++i) v.push_back(static_cast<int>(i));
            for (size_t i = 0; i < size; i += std::max<size_t>(1, size / 16)) {
                volatile int y = v[i]; (void)y;
            }
        }

        auto end = std::chrono::steady_clock::now();
        long long run_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            
        // Update stats (thread-safe)
        if (stats) {
            stats->count.fetch_add(1, std::memory_order_relaxed);
            stats->sum_us.fetch_add(run_us, std::memory_order_relaxed);
            long long prev_max = stats->max_us.load(std::memory_order_relaxed);
            while (run_us > prev_max && !stats->max_us.compare_exchange_weak(prev_max, run_us, std::memory_order_relaxed)) {
                // loop until max updated or prev_max >= run_us
            }
        }
    };
}

int main(int argc, char **argv)
{
    spdlog::set_level(spdlog::level::info);

    unsigned hw = std::thread::hardware_concurrency();
    if (hw == 0) hw = 1;

    // Tune these as needed for your pressure tests
    int numTasks = static_cast<int>(std::min<unsigned>(4, hw));
    float period_us = 1000.0f;  // 1 ms period -> 1 kHz tasks
    int policy = SCHED_FIFO;    // real-time policy (may require root)
    int priority = 99;          // high real-time priority

    spdlog::info("Starting {} stress tasks on up to {} logical CPUs (period={} us)", numTasks, hw, period_us);
    spdlog::info("Note: using SCHED_FIFO / high priority may require root privileges; if you see failures setting priority/affinity, re-run as root or switch to SCHED_OTHER.");

    std::vector<std::shared_ptr<ExecStats>> statsList;
    statsList.reserve(numTasks);
    std::vector<std::unique_ptr<PeriodicTask>> tasks;
    tasks.reserve(numTasks);
    for (int i = 0; i < numTasks; ++i) {
        std::stringstream ss;
        ss << "StressTask" << i;
        auto stats = std::make_shared<ExecStats>();
        statsList.push_back(stats);
        // bind the generated stress lambda
        tasks.emplace_back(std::make_unique<PeriodicTask>(ss.str(), period_us, makeStressFunction(i, period_us, stats), i % static_cast<int>(hw), policy, priority));
    }

    // Let the tasks run for a short pressure-run (tune as needed)
    std::this_thread::sleep_for(std::chrono::seconds(60));

    // Stop all tasks and report statistics
    for (auto &t : tasks) t->stop();

    for (size_t i = 0; i < tasks.size(); ++i) {
        long long count = statsList[i]->count.load();
        long long sum_us = statsList[i]->sum_us.load();
        long long max_us = statsList[i]->max_us.load();
        double avg = count ? static_cast<double>(sum_us) / static_cast<double>(count) : 0.0;
        long long count_p = statsList[i]->count_period.load();
        long long sum_p = statsList[i]->sum_period_us.load();
        long long max_p = statsList[i]->max_period_us.load();
        double avg_p = count_p ? static_cast<double>(sum_p) / static_cast<double>(count_p) : 0.0;

        spdlog::info("Task {} execs: count={}, avg_run={} us, max_run={} us", i, count, avg, max_us);
        spdlog::info("Task {} periods: samples={}, avg_period={} us, max_period={} us", i, count_p, avg_p, max_p);
    }

    spdlog::info("Stress run complete.");

    return 0;
}