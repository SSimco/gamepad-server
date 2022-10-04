#include "uinput.hpp"

#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <linux/uinput.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <tuple>

class error_from_errnum : public std::runtime_error {
 public:
  error_from_errnum(const std::string& msg, int errnum)
      : std::runtime_error(msg + std::string(strerror(errnum))) {}
};

class ioctl_error : public error_from_errnum {
 public:
  ioctl_error(int errnum) : error_from_errnum("ioctl error: ", errnum) {}
};
class write_error : public error_from_errnum {
 public:
  write_error(int errnum) : error_from_errnum("write error: ", errnum) {}
};
class file_descriptor_error : public std::runtime_error {
 public:
  file_descriptor_error()
      : std::runtime_error("file descriptor was not initialized") {}
};

template <typename... TArgs>
void _ioctl(std::optional<int>& fd, TArgs... args) {
  if (!fd.has_value()) {
    throw file_descriptor_error();
  }
  if (int errnum = ioctl(fd.value(), std::forward<TArgs>(args)...)) {
    throw ioctl_error(errnum);
  }
}

uinput::uinput(
    const std::string& name, const std::initializer_list<int>& ev_bits,
    const std::initializer_list<int>& prop_bits,
    const std::initializer_list<int>& buttons,
    const std::initializer_list<std::tuple<unsigned short, int, int>>& axes) {
  m_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

  for (auto&& evbit : ev_bits) {
    _ioctl(m_fd, UI_SET_EVBIT, evbit);
  }

  for (auto&& propbit : prop_bits) {
    _ioctl(m_fd, UI_SET_PROPBIT, propbit);
  }

  for (auto&& button : buttons) {
    _ioctl(m_fd, UI_SET_KEYBIT, button);
  }

  auto abs_setup = [&](unsigned short code, int min, int max) {
    _ioctl(m_fd, UI_SET_ABSBIT, code);

    struct uinput_abs_setup abs_setup = {
        .code = code,
        .absinfo = {.minimum = min, .maximum = max},
    };
    _ioctl(m_fd, UI_ABS_SETUP, &abs_setup);
  };

  for (auto it = axes.begin(); it != axes.end(); ++it) {
    unsigned short code;
    int min, max;
    std::tie(code, min, max) = *it;
    abs_setup(code, min, max);
  }
  struct uinput_setup setup = {.id = {
                                   .bustype = BUS_VIRTUAL,
                                   .vendor = 0x0123,
                                   .product = 0x0321,
                               }};
  std::copy_n(name.cbegin(),
              std::min(UINPUT_MAX_NAME_SIZE, static_cast<int>(name.size())),
              setup.name);
  _ioctl(m_fd, UI_DEV_SETUP, &setup);
  _ioctl(m_fd, UI_DEV_CREATE);
}

void uinput::emit_ev(unsigned short type, unsigned short code, int value) {
  if (!m_fd.has_value()) {
    throw file_descriptor_error();
  }

  struct input_event ev[2] = {{.type = type, .code = code, .value = value},
                              {.type = EV_SYN, .code = SYN_REPORT, .value = 0}};

  if (int errnum = write(m_fd.value(), &ev, sizeof(ev)); errnum < 0) {
    throw write_error(errnum);
  }
}

void uinput::emit_abs(unsigned short axis_code, int value) {
  emit_ev(EV_ABS, axis_code, value);
}

void uinput::emit_key(unsigned short key, int value) {
  emit_ev(EV_KEY, key, value);
}

uinput::~uinput() {
  if (m_fd.has_value()) {
    close(m_fd.value());
  }
}
