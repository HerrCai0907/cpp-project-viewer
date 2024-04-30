#pragma once

#include <cstdint>

namespace cpjview::persistence {

struct ErrorCode {
  explicit ErrorCode(std::uint16_t code) : m_code(code) {}
  std::uint16_t m_code;

  static ErrorCode ok() { return ErrorCode{200}; }
  static ErrorCode forbidden() { return ErrorCode{403}; }
  static ErrorCode not_found() { return ErrorCode{404}; }
};

} // namespace cpjview::persistence
