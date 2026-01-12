#include <dds/dds_publisher.hpp>
#include <idl/agilepf/LowState/LowState.hpp>
#include <idl/agilepf/LowState/LowStatePubSubTypes.hpp>
#include <cstring>

using namespace legged::dds;

int main(int argc, char** argv)
{
    int domain_id = 1;
    std::string network_interface = "lo";
    if(argc > 1)
    {
        domain_id = 0;
        network_interface = argv[1];
    }
    // Create DDS Publisher
    DDSPublisher<legged::idl::agilepf::LowState, legged::idl::agilepf::LowStatePubSubType> publisher(
        domain_id,         // domain ID
        network_interface, // network interface name
        "rt/lowstate",     // topic name
        2000.0f            // subscription period in us, 2ms
    );

    if (!publisher.init())
    {
        spdlog::error("Failed to initialize DDS Publisher.");
        return -1;
    }

    legged::idl::agilepf::LowState low_state_msg;

    while (true)
    {
        // Fill and publish a LowState message
        low_state_msg.header() = {0xAA, 0xBB};
        low_state_msg.tail() = {0xEE, 0xFF};
        for(int i = 0; i < 6; ++i) // damping control for 6 motors
        {
            low_state_msg.motor_state()[i].q() = 1.0f;
            low_state_msg.motor_state()[i].dq() = 2.0f;
        }
        publisher.publish(&low_state_msg);
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    return 0;
}