#include <dds/dds_publisher.hpp>
#include <idl/agilepf/LowCmd/LowCmd.hpp>
#include <idl/agilepf/LowCmd/LowCmdPubSubTypes.hpp>
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
    DDSPublisher<legged::idl::agilepf::LowCmd, legged::idl::agilepf::LowCmdPubSubType> publisher(
        domain_id,         // domain ID
        network_interface, // network interface name
        "rt/lowcmd",       // topic name
        2000.0f            // subscription period in us, 2ms
    );

    if (!publisher.init())
    {
        spdlog::error("Failed to initialize DDS Publisher.");
        return -1;
    }

    legged::idl::agilepf::LowCmd low_cmd_msg;

    while (true)
    {
        // Fill and publish a LowCmd message
        low_cmd_msg.header() = {0xAA, 0xBB};
        low_cmd_msg.tail() = {0xEE, 0xFF};
        for(int i = 0; i < 6; ++i) // damping control for 6 motors
        {
            low_cmd_msg.motor_cmd()[i].q() = 0.0f;
            low_cmd_msg.motor_cmd()[i].dq() = 0.0f;
            low_cmd_msg.motor_cmd()[i].tau() = 0.0f;
            low_cmd_msg.motor_cmd()[i].kp() = 0.0f;
            low_cmd_msg.motor_cmd()[i].kd() = 3.0f;
        }
        publisher.publish(&low_cmd_msg);
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    return 0;
}