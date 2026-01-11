#include <dds/dds_subscriber.hpp>
#include <idl/HelloWorld/HelloWorld.hpp>
#include <idl/HelloWorld/HelloWorldPubSubTypes.hpp>

using namespace legged::dds;

void message_callback(const void* msg)
{
    legged::idl::HelloWorld rev_msg = *(legged::idl::HelloWorld*)msg;
    if(rev_msg.index() % 1000 == 0) // log every 1000 messages(1s)
    {
        spdlog::info("Received message: index={}, message='{}', timestamp={}",
                 rev_msg.index(), rev_msg.message(), rev_msg.timestamp());
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
    DDSSubscriber<legged::idl::HelloWorld, legged::idl::HelloWorldPubSubType> subscriber(
        domain_id,         // domain ID
        network_interface, // network interface name
        "HelloWorldTopic", // topic name
        message_callback,  // callback function
        1000.0f            // subscription period in us
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