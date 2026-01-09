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

int main()
{
    // Create DDS Subscriber
    DDSSubscriber<legged::idl::HelloWorld, legged::idl::HelloWorldPubSubType> subscriber(
        1,                 // domain ID
        "lo",              // network interface name
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