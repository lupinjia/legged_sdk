#include <dds/dds_publisher.hpp>
#include <idl/HelloWorld/HelloWorld.hpp>
#include <idl/HelloWorld/HelloWorldPubSubTypes.hpp>

using namespace legged::dds;

int main()
{
    // Create DDS Publisher
    DDSPublisher<legged::idl::HelloWorld, legged::idl::HelloWorldPubSubType> publisher(
        1,                 // domain ID
        "lo",              // network interface name
        "HelloWorldTopic", // topic name
        1000.0f            // publication period in us
    );

    if (!publisher.init())
    {
        spdlog::error("Failed to initialize DDS Publisher.");
        return -1;
    }

    legged::idl::HelloWorld msg;
    msg.message("Hello, DDS!");
    int index = 0;

    while (true)
    {
        msg.index(index++);
        msg.timestamp(static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count()));
        if (publisher.publish(&msg))
        {
            if(index % 1000 == 0) // log every 1000 messages(1s)
            {
                spdlog::info("Publish: index={}, message='{}', timestamp={}", index, msg.message(), msg.timestamp());
            }
        }
        else
        {
            spdlog::warn("No subscribers matched, message not published.");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return 0;
}