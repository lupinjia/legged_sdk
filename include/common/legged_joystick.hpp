// This file is based on https://github.com/unitreerobotics/unitree_sdk2/blob/main/include/unitree/dds_wrapper/common/unitree_joystick.hpp

#pragma once

#include <math.h>
#include <stdint.h>
#include <iostream>
#include <chrono>

namespace legged
{
namespace common
{

typedef struct
{
  float lx;
  float ly;
  float rx;
  float ry;
  uint8_t start;
  uint8_t select;
  uint8_t left;
  uint8_t right;
  uint8_t up;
  uint8_t down;
  uint8_t a;
  uint8_t b;
  uint8_t x;
  uint8_t y;
  uint8_t l1;
  uint8_t r1;
}JoystickDataStruct;

// ***************************** Button & Axis ***************************** //
class KeyBase
{
/**
 * Example:
 * >>> btn = KeyBase();
 * 
 * >>> btn(1); // update
 * >>> btn.pressed
 * >>> if btn.on_pressed:
 * >>>   print(btn.click_cnt) 
 * >>> if btn.pressed:
 * >>>   print(btn.pressed_time)
 */
public:
  KeyBase() = default;

  bool pressed = false;
  bool on_pressed = false;
  bool on_released = false;
  int click_cnt = 0; // used at `on_pressed`
  float pressed_time = 0.0;

protected:
  void update(bool is_pressed)
  {
    pressed = is_pressed;
    on_pressed = (pressed && !last_pressed_);
    on_released = (!pressed && last_pressed_);
    last_pressed_ = pressed;

    auto now = std::chrono::steady_clock::now();
    if (on_pressed)
    {
      if (std::chrono::duration<float>(now - last_click_time_).count() < double_click_threshold_) {
        click_cnt++;
      } else {
        click_cnt = 1;
      }
      last_click_time_ = now;
    }

    if(pressed) {
      pressed_time = std::chrono::duration<float>(now - last_click_time_).count();
    }
    if (!(pressed || on_released)) { // save once time on released
      pressed_time = 0.0;
    }
  }
private:
  bool last_pressed_{false};
  std::chrono::steady_clock::time_point last_click_time_{};
  const float double_click_threshold_{0.5}; // Unit: second
};

template <typename T> // int, bool, string
class Button : public KeyBase
{
public:
  void operator()(const T& data){
    bool is_pressed = (data != dataNull_);
    update(is_pressed);
    data_ = data;
  }
  const T& operator()() { return data_; }
private:
  T data_{}, dataNull_{};
};

class Axis : public KeyBase
{
public:
  void operator()(const float& data){ // update
    auto data_deadzone = std::fabs(data) < deadzone ? 0.0 : data;
    float new_data = data_ * (1.0 - smooth) + data_deadzone * smooth;
    bool is_pressed = (new_data > threshold);
    update(is_pressed);
    data_ = new_data;
  }

  const float& operator()() { return data_; }

  float smooth = 0.03;
  float deadzone = 0.01;

  float threshold{0.5}; // Change an axis value to a button
private:
  float data_{};
};

// ***************************** Legged Joystick Interface ***************************** //

class LeggedJoystick
{
public:
  LeggedJoystick() = default;
  
  // Adopts standard joystick key names
  Button<int> select;
  Button<int> start;
  Button<int> L1;
  Button<int> R1;
  Button<int> A;
  Button<int> B;
  Button<int> X;
  Button<int> Y;
  Button<int> up;
  Button<int> down;
  Button<int> left;
  Button<int> right;
  Axis lx;
  Axis ly;
  Axis rx;
  Axis ry;

  virtual void update(){};

  void extract(const JoystickDataStruct& joystick_data)
  {
    select(joystick_data.select);
    start(joystick_data.start);
    L1(joystick_data.l1);
    R1(joystick_data.r1);
    A(joystick_data.a);
    B(joystick_data.b);
    X(joystick_data.x);
    Y(joystick_data.y);
    up(joystick_data.up);
    down(joystick_data.down);
    left(joystick_data.left);
    right(joystick_data.right);
    lx(joystick_data.lx);
    ly(joystick_data.ly);
    rx(joystick_data.rx);
    ry(joystick_data.ry);
  }

  JoystickDataStruct combine()
  {
    JoystickDataStruct joystick_data;
    joystick_data.select = select();
    joystick_data.start = start();
    joystick_data.l1 = L1();
    joystick_data.r1 = R1();
    joystick_data.a = A();
    joystick_data.b = B();
    joystick_data.x = X();
    joystick_data.y = Y();
    joystick_data.up = up();
    joystick_data.down = down();
    joystick_data.left = left();
    joystick_data.right = right();
    joystick_data.lx = lx();
    joystick_data.ly = ly();
    joystick_data.rx = rx();
    joystick_data.ry = ry();
    return joystick_data;
  }
};

} // namespace common
} // namespace agilepf