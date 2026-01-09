/**
 * @file dds_subscriber.hpp
 * @author lupinjia (jason_1120202397@163.com)
 * @brief  Defines a DDS Subscriber class optimized for real-time UDP communication.
 * @version 0.1
 * @date 2026-01-05
 * 
 * @copyright Copyright (c) 2026 Beijing Institute of Technology, Yasen Jia
 * 
 */
#pragma once
// fast-dds includes
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>
// spdlog includes
#include <spdlog/spdlog.h>
// std includes
#include <chrono>
#include <thread>
#include <iostream>
#include <cstdint>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

namespace legged
{
namespace dds
{

/**
 * @brief DDS Subscriber designed for real-time UDP communication
 *        Default qos settings are optimized for low-latency data transmission.
 *
 * @attention This class is for high-frequency data subscription over UDP (>= 500Hz). 
 *            Performance for lower frequency subscription is not guaranteed.
 * @tparam MSG  message type
 * @tparam MSG_TYPE_SUPPORT  type support for the message type
 */
template<typename MSG, typename MSG_TYPE_SUPPORT>
class DDSSubscriber : public DataReaderListener
{
public:
    using MsgType = MSG;
    /**
     * @brief Construct a new DDSSubscriber object
     * 
     * @param domain_id          DDS domain ID
     * @param interface_name     Network interface name
     * @param topic_name         DDS topic name
     * @param callback_function  Callback function to process received messages
     * @param period_us          Subscription period in microseconds (for sepcifying deadline in Qos)
     */
    DDSSubscriber(int domain_id, std::string interface_name,
                 std::string topic_name, 
                 std::function<void(const void*)> callback_function, 
                 float period_us = 1000.0)
        : participant_(nullptr)
        , subscriber_(nullptr)
        , topic_(nullptr)
        , reader_(nullptr)
        , period_us_(period_us)
        , type_(new MSG_TYPE_SUPPORT())
        , interface_name_(interface_name)
        , domain_id_(domain_id)
        , topic_name_(topic_name)
        , callback_function_(callback_function)
    {
    }

    /**
     * @brief Destroy the DDSSubscriber object
     * 
     */
    ~DDSSubscriber()
    {
        if (reader_ != nullptr)
        {
            subscriber_->delete_datareader(reader_);
        }
        if (subscriber_ != nullptr)
        {
            participant_->delete_subscriber(subscriber_);
        }
        if (topic_ != nullptr)
        {
            participant_->delete_topic(topic_);
        }
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }

    /**
     * @brief initialize the DDS Subscriber
     * 
     * @return true 
     * @return false 
     */
    bool init()
    {
        // Create realtime DomainParticipant with custom UDP transport
        createRealtimeParticipant();
        if (participant_ == nullptr)
        {
            spdlog::error("Error creating DomainParticipant!");
            return false;
        }

        // Register the data type
        type_.register_type(participant_);

        // Create Topic
        topic_ = participant_->create_topic(
            topic_name_,
            type_.get_type_name(),
            TOPIC_QOS_DEFAULT);
        if (topic_ == nullptr)
        {
            spdlog::error("Error creating Topic!");
            return false;
        }

        // Create Subscriber
        subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
        if (subscriber_ == nullptr)
        {
            spdlog::error("Error creating Subscriber!");
            return false;
        }

        // Create DataReader
        createRealtimeDatareader();
        if (reader_ == nullptr)
        {
            spdlog::error("Error creating DataReader!");
            return false;
        }

        spdlog::info("DDS Subscriber initialized successfully on topic: {}, with domain_id: {}, interface: {}", 
                          topic_name_, domain_id_, interface_name_);

        return true;
    }

    void on_data_available(DataReader* reader) override
    {
        SampleInfo info;
        MSG data;
        
        if (reader->take_next_sample(&data, &info) == RETCODE_OK)
        {
            if (info.valid_data)
            {
                callback_function_(&data);
            }
        }
    }

private:
    DomainParticipant* participant_;
    Subscriber* subscriber_;
    Topic* topic_;
    DataReader* reader_;
    TypeSupport type_;
    float period_us_;
    std::string interface_name_;
    int domain_id_;
    std::string topic_name_;
    // Qos
    DomainParticipantQos participant_qos_;
    DataReaderQos reader_qos_;
    std::function<void(const void*)> callback_function_;

    void createRealtimeParticipant()
    {
        participant_qos_.name("Subscriber, domain_id: " + std::to_string(domain_id_)
            + ", interface: " + interface_name_ + ", topic: " + topic_name_);
        // disable built-in transports
        participant_qos_.transport().use_builtin_transports = false;
        // create and configure UDPv4 transport
        auto udp_transport = std::make_shared<UDPv4TransportDescriptor>();
        udp_transport->sendBufferSize = 65536;
        udp_transport->receiveBufferSize = 65536;
        udp_transport->TTL = 1;
        udp_transport->non_blocking_send = true;
        udp_transport->interfaceWhiteList.emplace_back(interface_name_); // select network interface
        // set user transports
        participant_qos_.transport().user_transports.push_back(udp_transport);
        // configure RTPS properties
        participant_qos_.wire_protocol().builtin.discovery_config.leaseDuration = 
            Duration_t(1, 0);
        participant_qos_.wire_protocol().builtin.discovery_config.leaseDuration_announcementperiod = 
            Duration_t(0, 100 * 1000000); // 100 ms
        // create DomainParticipant, domainId can be set to 0 or any number
        participant_ = DomainParticipantFactory::get_instance()->create_participant(
            domain_id_, participant_qos_);
    }

    void createRealtimeDatareader()
    {
        DataReaderQos reader_qos_;
        // Reliability set to best effort (reduce latency)
        reader_qos_.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;
        // Durability set to volatile (only care about latest data)
        reader_qos_.durability().kind = VOLATILE_DURABILITY_QOS;
        // Set history depth to 1 (only keep latest data)
        reader_qos_.history().kind = KEEP_LAST_HISTORY_QOS;
        reader_qos_.history().depth = 1;
        // Set resource limits
        reader_qos_.resource_limits().max_samples = 1;
        reader_qos_.resource_limits().max_instances = 1;
        reader_qos_.resource_limits().max_samples_per_instance = 1;
        // Set deadline (data validity period)
        reader_qos_.deadline().period = {0, static_cast<uint32_t>(period_us_ * 1000)};  // convert to nanoseconds
        // Set the maximum size for writer matched resource limits collection to 1 and its allocation configuration to fixed size
        // only one publisher is expected to publish data to this subscriber
        reader_qos_.reader_resource_limits().matched_publisher_allocation = 
            eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(1u);
        reader_ = subscriber_->create_datareader(topic_, reader_qos_, this);
    }
};

template<typename MSG, typename MSG_TYPE_SUPPORT>
using DDSSubscriberPtr = std::shared_ptr<DDSSubscriber<MSG, MSG_TYPE_SUPPORT>>;

}
}