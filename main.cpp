#include <linux/input-event-codes.h>
#include <unistd.h>

#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

#include "gamepad.hpp"
#include "udp_client.hpp"

template <typename T, size_t array_size, size_t buffer_size>
void copy_from_buffer(std::array<T, array_size>& array,
                      const boost::array<char, buffer_size>& buffer,
                      size_t& index) {
  const T* data;
  data = reinterpret_cast<const T*>(buffer.data() + index);
  std::copy(data, data + array_size, array.begin());
  index += array_size * sizeof(T);
}

enum class input_type : char { key = 0x01, gyro = 0x02, axis = 0x03 };

int main(int argc, char* argv[]) {
  gamepad pad;
  auto on_msg = [&](const boost::array<char, udp_buffer_size>& buffer) {
    size_t index = 0;
    input_type in_type = static_cast<input_type>(buffer[index++]);
    switch (in_type) {
      case input_type::key: {
        unsigned short key =
            *reinterpret_cast<const unsigned short*>(buffer.data() + index);
        index += sizeof(unsigned short);
        int val =
            *reinterpret_cast<const unsigned short*>(buffer.data() + index);
        index += sizeof(int);
        pad.emit_key(key, val);
        break;
      }
      case input_type::axis: {
        unsigned short axis =
            *reinterpret_cast<const unsigned short*>(buffer.data() + index);
        index += sizeof(unsigned short);
        int val = *reinterpret_cast<const int*>(buffer.data() + index);
        index += sizeof(int);
        pad.emit_axis(axis, val);
        break;
      }
      case input_type::gyro: {
        std::array<int, 3> gyro, accel;
        copy_from_buffer(gyro, buffer, index);
        copy_from_buffer(accel, buffer, index);
        pad.emit_gyro(gyro, accel);
        break;
      }
      default:
        std::cerr << "Unkown input type: " << static_cast<int>(in_type)
                  << std::endl;
    }
  };
  const unsigned short defaultPort = 15366;
  unsigned short port;
  if (argc > 1) {
    try {
      port = std::stoi(argv[1]);
    } catch (std::invalid_argument inv_arg) {
      std::cerr << "Bad port: " << argv[1] << std::endl;
      port = defaultPort;
    }
  } else {
    port = defaultPort;
  }
  udp_client client(port, on_msg);
  client.recieve();
}
