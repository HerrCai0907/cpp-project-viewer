#pragma once

#include <string>

namespace cpjview::protocol {

struct Code {
  std::string m_name;
  std::string m_code;

  static Code from_json(std::string json_str);
  std::string to_json() const;
};

} // namespace cpjview::protocol
