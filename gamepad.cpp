#include "gamepad.hpp"

#include <linux/input-event-codes.h>

#include <tuple>

static const int axis_value = 32767;

gamepad::gamepad(const std::string& controller_name)
    : m_gamepad(controller_name + " - gamepad", {EV_KEY, EV_ABS}, {},
                {BTN_A, BTN_B, BTN_X, BTN_Y, BTN_TL, BTN_TR, BTN_TL2, BTN_TR2,
                 BTN_SELECT, BTN_START, BTN_DPAD_UP, BTN_DPAD_DOWN,
                 BTN_DPAD_LEFT, BTN_DPAD_RIGHT},
                {{ABS_X, -axis_value, axis_value},
                 {ABS_Y, -axis_value, axis_value},
                 {ABS_RX, -axis_value, axis_value},
                 {ABS_RY, -axis_value, axis_value}}),
      m_accelerometer(controller_name + " - accelerometer", {EV_ABS},
                      {INPUT_PROP_ACCELEROMETER}, {},
                      {{ABS_X, -axis_value, axis_value},
                       {ABS_Y, -axis_value, axis_value},
                       {ABS_Z, -axis_value, axis_value},
                       {ABS_RX, -axis_value, axis_value},
                       {ABS_RY, -axis_value, axis_value},
                       {ABS_RZ, -axis_value, axis_value}}) {}

void gamepad::emit_gyro(const std::array<int, 3>& gyro,
                           const std::array<int, 3>& accel) {
  m_accelerometer.emit_abs(ABS_RX, gyro[0]);
  m_accelerometer.emit_abs(ABS_RY, gyro[1]);
  m_accelerometer.emit_abs(ABS_RZ, gyro[2]);

  m_accelerometer.emit_abs(ABS_X, accel[0]);
  m_accelerometer.emit_abs(ABS_Y, accel[1]);
  m_accelerometer.emit_abs(ABS_Z, accel[2]);
}

void gamepad::emit_key(unsigned short key, int value) {
  m_gamepad.emit_key(key, value);
}

void gamepad::emit_axis(unsigned short axis, int value) {
  m_gamepad.emit_abs(axis, value);
}

void gamepad::sync() {
  m_gamepad.emit_ev(EV_SYN, SYN_REPORT, 0);
  m_accelerometer.emit_ev(EV_SYN, SYN_REPORT, 0);
}
