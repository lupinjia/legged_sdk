// This file is based on https://github.com/unitreerobotics/unitree_sdk2/blob/f29ee9f234851e9e79f75102c0f9e83008d8fdd1/include/unitree/dds_wrapper/common/Publisher.h

#pragma once

#include <dds/dds_publisher.hpp>
#include <atomic>
#include <thread>
#include <memory>

namespace legged
{
namespace dds_wrapper
{

/**
 * @brief Base class for convenient publishing to topics.
 * This class provides an easy-to-use interface for publishing messages to topics.
 *
 * @tparam MessageType The type of message being published.
 */
template<typename MSG, typename MSG_TYPE_SUPPORT>
class PublisherBase : public legged::dds::DDSPublisher<MSG, MSG_TYPE_SUPPORT>
{
public:
  using MsgType = MSG;
  using MsgTypeSupport = MSG_TYPE_SUPPORT;
  using SharedPtr = std::shared_ptr<PublisherBase<MsgType, MsgTypeSupport>>;

  PublisherBase(int domain_id, std::string interface_name, 
                std::string topic_name, float period_us)
  : legged::dds::DDSPublisher<MSG, MSG_TYPE_SUPPORT>(domain_id, interface_name, topic_name, period_us)
  {
    this->init();
  }
};

// For details: see https://github.com/ros-controls/realtime_tools
template <typename MSG, typename MSG_TYPE_SUPPORT>
class RealTimePublisher
{
public:
  using MsgType = MSG;
  using MsgTypeSupport = MSG_TYPE_SUPPORT;
  using PublisherSharedPtr = typename legged::dds::DDSPublisherPtr<MsgType, MsgTypeSupport>;

  MsgType msg_{};

  explicit RealTimePublisher(PublisherSharedPtr publisher)
  : publisher_(publisher), is_running_(false), keep_running_(true), turn_(LOOP_NOT_STARTED)
  {
    thread_ = std::thread(&RealTimePublisher::publishingLoop, this);
  }

 /**
  * @brief Constructs a RealTimePublisher with specified DDS domain configuration.
  * 
  * @param domain_id The DDS domain ID for publisher communication.
  * @param interface_name The network interface name to bind the publisher to.
  * @param topic_name The DDS topic name for publishing messages.
  * @param period_us The publishing period in microseconds.
  * 
  * @note This constructor delegates to the main constructor by creating a shared_ptr
  *       to PublisherBase with the provided parameters.
  */
  explicit RealTimePublisher(int domain_id, std::string interface_name, 
                             std::string topic_name, float period_us)
  : RealTimePublisher(std::make_shared<PublisherBase<MsgType, MsgTypeSupport>>(
                        domain_id, interface_name, topic_name, period_us))
  {}

  ~RealTimePublisher()
  {
    stop();
    while (is_running()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    if(thread_.joinable()) { thread_.join(); }
  }

  void stop()
  {
    keep_running_ = false;
  }

  /**
   * @brief Try to get the data lock from realtime
   * 
   * To publish data from the realtime loop, you need to run trylock to
   * attend to get unique access to the msg_ variable. Trylock returns
   * true if the lock was aquired, and false otherwise.
   */
  bool trylock()
  {
    if(mutex_.try_lock())
    {
      if(turn_ == REALTIME) {
        return true;
      } else {
        mutex_.unlock();
        return false;
      }
    } else {
      return false;
    }
  }

  /**
   * @brief Unlock the msg_ variable and publish it.
   */
  void unlockAndPublish() 
  {
    turn_ = NON_REALTIME;
    mutex_.unlock();
  }

  /**
   * @brief Get the data lock from non-realtime.
   * 
   * To publish data from the realtime loop, you need to run trylock to
   * attenot to get unique access to the msg_ variable. Teylock returns
   * true if the lock was aquired, and false otherwise.
   */
  void lock()
  {
    // never actually lock on the lock
    while (!mutex_.try_lock()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }

  /**
   * @brief Unlocks the data without publishing anything.
   */
  void unlock() { mutex_.unlock(); }

protected:
  virtual void pre_communication() {}  // something before sending the message
  virtual void post_communication() {} // something after sending the message

private:
  // non-copyable
  RealTimePublisher(const RealTimePublisher&) = delete;
  RealTimePublisher& operator=(const RealTimePublisher&) = delete;


  bool is_running() const { return is_running_; }

  void publishingLoop()
  {
    is_running_ = true;
    turn_ = REALTIME;

    while (keep_running_)
    {
      MsgType outgoing;

      // Locks msg_ and copies it
      lock();
      while (turn_ != NON_REALTIME && keep_running_)
      {
        unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        lock();
      }
      pre_communication();
      outgoing = msg_;
      turn_ = REALTIME;

      unlock();

      if(keep_running_) { 
        publisher_->publish(&outgoing); 
      }
      post_communication();
    }
    is_running_ = false;
  }

  PublisherSharedPtr publisher_;
  std::atomic_bool is_running_;
  std::atomic_bool keep_running_;

  std::mutex mutex_;

  std::thread thread_;

  enum { REALTIME, NON_REALTIME, LOOP_NOT_STARTED };
  std::atomic<int> turn_;
};

} // namespace robot
} // namespace unitree