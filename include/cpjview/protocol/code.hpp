#pragma once

#include <string>

namespace cpjview::protocol {

struct SourceCode {
  std::string m_name;
  std::string m_code;

  static SourceCode from_json(std::string json_str);
  std::string to_json() const;
};

} // namespace cpjview::protocol
