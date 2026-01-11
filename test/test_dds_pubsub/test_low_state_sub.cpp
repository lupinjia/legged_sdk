#include <dds/dds_subscriber.hpp>
#include <idl/agilepf/LowState/LowState.hpp>
#include <idl/agilepf/LowState/LowStatePubSubTypes.hpp>
#include <common/crc.h>
#include <cstring>

using namespace legged::dds;

void message_callback(const void* msg)
{
    uint32_t offset = 0;
    legged::idl::agilepf::LowState rev_msg = *(legged::idl::agilepf::LowState*)msg;
    if(rev_msg.header()[0] == 0xAA && rev_msg.header()[1] == 0xBB && rev_msg.tail()[0] == 0xEE && rev_msg.tail()[1] == 0xFF)
    {
        spdlog::info("LowState message received.");
        spdlog::info("Quaternion: [{}, {}, {}, {}]", rev_msg.imu_state().quaternion()[0], rev_msg.imu_state().quaternion()[1],
                rev_msg.imu_state().quaternion()[2], rev_msg.imu_state().quaternion()[3]);
        spdlog::info("Motor Pos: [{}, {}, {}, {}, {}, {}]", 
                rev_msg.motor_state()[0].q(),
                rev_msg.motor_state()[1].q(),
                rev_msg.motor_state()[2].q(),
                rev_msg.motor_state()[3].q(),
                rev_msg.motor_state()[4].q(),
                rev_msg.motor_state()[5].q());
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
    DDSSubscriber<legged::idl::agilepf::LowState, legged::idl::agilepf::LowStatePubSubType> subscriber(
        domain_id,         // domain ID
        network_interface, // network interface name
        "rt/lowstate",     // topic name
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