#ifndef __UINPUT_HPP_
#define __UINPUT_HPP_

#include <initializer_list>
#include <optional>
#include <string>

class uinput {
 private:
  std::optional<int> m_fd;

 public:
  uinput() = default;
  uinput(const uinput&) = delete;

  uinput(
      const std::string& name, const std::initializer_list<int>& ev_bits,
      const std::initializer_list<int>& prop_bits,
      const std::initializer_list<int>& buttons,
      const std::initializer_list<std::tuple<unsigned short, int, int>>& axes);
  void emit_ev(unsigned short type, unsigned short code, int value);
  void emit_abs(unsigned short axis_code, int value);
  void emit_key(unsigned short key, int value);

  ~uinput();
};

#endif
