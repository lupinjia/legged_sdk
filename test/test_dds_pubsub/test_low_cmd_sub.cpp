#include <dds/dds_subscriber.hpp>
#include <idl/agilepf/LowCmd/LowCmd.hpp>
#include <idl/agilepf/LowCmd/LowCmdPubSubTypes.hpp>
#include <cstring>

using namespace legged::dds;

void message_callback(const void* msg)
{
    uint32_t offset = 0;
    legged::idl::agilepf::LowCmd rev_msg = *(legged::idl::agilepf::LowCmd*)msg;
    if(rev_msg.header()[0] == 0xAA && rev_msg.header()[1] == 0xBB && rev_msg.tail()[0] == 0xEE && rev_msg.tail()[1] == 0xFF)
    {
        spdlog::info("LowCmd message received.");
        spdlog::info("Motor Pos Des: [{}, {}, {}, {}, {}, {}]", 
                rev_msg.motor_cmd()[0].q(),
                rev_msg.motor_cmd()[1].q(),
                rev_msg.motor_cmd()[2].q(),
                rev_msg.motor_cmd()[3].q(),
                rev_msg.motor_cmd()[4].q(),
                rev_msg.motor_cmd()[5].q());
        spdlog::info("Motor Kp and Kd: [{}, {}]",
                rev_msg.motor_cmd()[0].kp(),
                rev_msg.motor_cmd()[0].kd());
    }
}

int main(int argc, char** argv)
{
    int domain_id = 1;
    std::string network_interface = "lo";
    if(argc > 1)
    {
        domain_id = 0;
        network_interface = argv[1];
    }
    // Create DDS Subscriber
    DDSSubscriber<legged::idl::agilepf::LowCmd, legged::idl::agilepf::LowCmdPubSubType> subscriber(
        domain_id,         // domain ID
        network_interface, // network interface name
        "rt/lowcmd",       // topic name
        message_callback,  // callback function
        2000.0f            // subscription period in us
    );

    if (!subscriber.init())
    {
        spdlog::error("Failed to initialize DDS Subscriber.");
        return -1;
    }

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}