/**
 * @file dds_publisher.hpp
 * @author lupinjia (jason_1120202397@163.com)
 * @brief  Defines a DDS Publisher class optimized for real-time UDP communication.
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
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
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
 * @brief DDS Publisher designed for real-time UDP communication
 *        Default qos settings are optimized for low-latency data transmission.
 *
 * @attention This class is for high-frequency data publishing over UDP (>= 500Hz). 
 *            Performance for lower frequency publishing is not guaranteed.
 * @tparam MSG  message type
 * @tparam MSG_TYPE_SUPPORT  type support for the message type
 */
template<typename MSG, typename MSG_TYPE_SUPPORT>
class DDSPublisher
{
public:
    /**
     * @brief Construct a new DDSPublisher object
     * 
     * @param domain_id       DDS domain ID
     * @param interface_name  Network interface name
     * @param topic_name      DDS topic name
     * @param period_us       Publication period in microseconds
     */
    DDSPublisher(int domain_id, std::string interface_name,
                 std::string topic_name, float period_us = 1000.0)
        : participant_(nullptr)
        , publisher_(nullptr)
        , topic_(nullptr)
        , writer_(nullptr)
        , period_us_(period_us)
        , type_(new MSG_TYPE_SUPPORT())
        , interface_name_(interface_name)
        , domain_id_(domain_id)
        , topic_name_(topic_name)
    {
    }

    /**
     * @brief Destroy the DDSPublisher object
     * 
     */
    ~DDSPublisher()
    {
        if (writer_ != nullptr)
        {
            publisher_->delete_datawriter(writer_);
        }
        if (publisher_ != nullptr)
        {
            participant_->delete_publisher(publisher_);
        }
        if (topic_ != nullptr)
        {
            participant_->delete_topic(topic_);
        }
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
        spdlog::info("DDS Publisher destroyed.");
    }

    /**
     * @brief initialize the DDS Publisher
     * 
     * @return true  if success
     * @return false if failed
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

        // Create Publisher
        publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
        if (publisher_ == nullptr)
        {
            spdlog::error("Error creating Publisher!");
            return false;
        }

        // Create DataWriter
        createRealtimeDatawriter();
        if (writer_ == nullptr)
        {
            spdlog::error("Error creating DataWriter!");
            return false;
        }
        return true;
    }

    /**
     * @brief Publish a message
     * 
     * @param  msg     message to be published
     * @return true   if published successfully
     * @return false  if failed
     */
    bool publish(const void* msg)
    {
        if (writer_ == nullptr)
        {
            spdlog::error("DataWriter is not initialized!");
            return false;
        }
        // no need to check matched subscribers for real-time publisher
        writer_->write(msg);
        return true;
    }

private:
    DomainParticipant* participant_;
    Topic* topic_;
    Publisher* publisher_;
    DataWriter* writer_;
    TypeSupport type_;
    float period_us_;
    std::string interface_name_;
    int domain_id_;
    std::string topic_name_;
    // Qos
    DomainParticipantQos participant_qos_;
    DataWriterQos writer_qos_;

    void createRealtimeParticipant()
    {
        participant_qos_.name("Publisher, domain_id: " + std::to_string(domain_id_)
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

    void createRealtimeDatawriter()
    {
        // Reliability set to best effort (reduce latency)
        writer_qos_.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;
        // Durability set to volatile (only care about latest data)
        writer_qos_.durability().kind = VOLATILE_DURABILITY_QOS;
        // Set history depth to 1 (only keep latest data)
        writer_qos_.history().kind = KEEP_LAST_HISTORY_QOS;
        writer_qos_.history().depth = 1;
        // Set resource limits
        writer_qos_.resource_limits().max_samples = 1;
        writer_qos_.resource_limits().max_instances = 1;
        writer_qos_.resource_limits().max_samples_per_instance = 1;
        // Set deadline (data validity period)
        writer_qos_.deadline().period = {0, static_cast<uint32_t>(period_us_ * 1000)};  // convert to nanoseconds
        // Set latency budget (target latency)
        writer_qos_.latency_budget().duration = {0, static_cast<uint32_t>(period_us_ * 1000 / 10)};  // target latency is one-tenth of the period
        // Use synchronous publish mode, blocking in this thread until send is complete. refer to https://discourse.openrobotics.org/t/rmw-fast-dds-publication-mode-sync-vs-async-and-how-to-change-it/17153
        writer_qos_.publish_mode().kind = SYNCHRONOUS_PUBLISH_MODE;
        // Set liveliness
        writer_qos_.liveliness().kind = AUTOMATIC_LIVELINESS_QOS;
        writer_qos_.liveliness().lease_duration = {1, 0};  // 1 second
        writer_qos_.liveliness().announcement_period = {0, 100000000};  // 100ms
        writer_ = publisher_->create_datawriter(topic_, writer_qos_, nullptr);
    }
};

template<typename MSG, typename MSG_TYPE_SUPPORT>
using DDSPublisherPtr = std::shared_ptr<DDSPublisher<MSG, MSG_TYPE_SUPPORT>>;

}
}