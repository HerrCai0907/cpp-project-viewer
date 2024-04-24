#pragma once

#include <string>

namespace cpjview::protocol {

struct Inheritance {
  std::string m_derived;
  std::string m_base;

  static Inheritance from_json(std::string json_str);
  std::string to_json() const;
};

} // namespace cpjview::protocol
