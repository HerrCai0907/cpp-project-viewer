#pragma once

#include <string>

namespace cpjview::protocol {

struct Project {
  std::string m_name;

  static Project from_json(std::string json_str);
  std::string to_json() const;
};

} // namespace cpjview::protocol
