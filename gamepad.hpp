#ifndef __CONTROLLER_HPP_
#define __CONTROLLER_HPP_

#include <array>

#include "uinput.hpp"

class gamepad {
 private:
  uinput m_gamepad;
  uinput m_accelerometer;
  void sync();

 public:
  gamepad(const std::string& controller_name = "Controller");
  void emit_gyro(const std::array<int, 3>& gyro,
                 const std::array<int, 3>& accel);
  void emit_key(unsigned short key, int value);
  void emit_axis(unsigned short axis, int value);
};

#endif