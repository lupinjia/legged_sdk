#pragma once

#include "dds_wrapper/common/publisher.h"
#include "common/crc.h"
#include "common/legged_joystick.hpp"

#include <idl/agilepf/LowCmd/LowCmd.hpp>
#include <idl/agilepf/LowCmd/LowCmdPubSubTypes.hpp>
#include <idl/agilepf/LowState/LowState.hpp>
#include <idl/agilepf/LowState/LowStatePubSubTypes.hpp>
#include <idl/agilepf/JoystickData/JoystickData.hpp>
#include <idl/agilepf/JoystickData/JoystickDataPubSubTypes.hpp>

namespace legged
{
namespace dds_wrapper
{
namespace agilepf
{
namespace publisher
{

class LowCmdPublisher : public RealTimePublisher<legged::idl::agilepf::LowCmd, 
                                       legged::idl::agilepf::LowCmdPubSubType>
{
public:
  LowCmdPublisher(int domain_id, std::string interface_name, 
                  float period_us, std::string topic_name = "rt/lowcmd")
  : RealTimePublisher<MsgType, MsgTypeSupport>(domain_id, interface_name, 
                                                  topic_name, period_us) 
  {
    msg_.header() = {0xAA, 0xBB};
    msg_.tail() = {0xEE, 0xFF};

  } 

private:
  /**
   * @brief Something before sending the message.
   */
  void pre_communication() override {
    // tail 2 bytes, crc 4 bytes
    // data in between is sizeof(MsgType) - 6 bytes
    msg_.crc() = crc32_core((uint32_t*)&msg_, (sizeof(MsgType)-6));
  }
};


class LowStatePublisher : public RealTimePublisher<legged::idl::agilepf::LowState, 
                                     legged::idl::agilepf::LowStatePubSubType>
{
public:
  LowStatePublisher(int domain_id, std::string interface_name, 
          float period_us, std::string topic_name = "rt/lowstate")
  : RealTimePublisher<MsgType, MsgTypeSupport>(domain_id, interface_name, 
                                                   topic_name, period_us) 
  {}

  std::shared_ptr<legged::common::LeggedJoystick> joystick = nullptr;

private:
  void pre_communication() override {
    if (joystick) {
      legged::common::JoystickDataStruct data = joystick->combine();
      memcpy(&msg_.joystick_data(), &data, sizeof(legged::common::JoystickDataStruct));
    }
    msg_.crc() = crc32_core((uint32_t*)&msg_, sizeof(MsgType)-6);
  }
};

class JoystickPublisher : public RealTimePublisher<legged::idl::agilepf::JoystickData, 
                                       legged::idl::agilepf::JoystickDataPubSubType>
{
public:
  JoystickPublisher(int domain_id, std::string interface_name, 
          float period_us, std::string topic_name = "rt/joystick")
  : RealTimePublisher<MsgType, MsgTypeSupport>(domain_id, interface_name, 
                                                  topic_name, period_us) 
  {
  }

  std::shared_ptr<legged::common::LeggedJoystick> joystick = nullptr;

private:
  void pre_communication() override {
    if (joystick) {
      msg_.left_stick_x() = joystick->lx();
      msg_.left_stick_y() = joystick->ly();
      msg_.right_stick_x() = joystick->rx();
      msg_.right_stick_y() = joystick->ry();
      msg_.start_button() = joystick->start();
      msg_.select_button() = joystick->select();
      msg_.left_button() = joystick->left();
      msg_.right_button() = joystick->right();
      msg_.up_button() = joystick->up();
      msg_.down_button() = joystick->down();
      msg_.a_button() = joystick->A();
      msg_.b_button() = joystick->B();
      msg_.x_button() = joystick->X();
      msg_.y_button() = joystick->Y();
      msg_.l1_button() = joystick->L1();
      msg_.r1_button() = joystick->R1();
    }
  }
};

} // namespace publisher
} // namespace agilepf
} // namespace dds_wrapper
} // namespace legged