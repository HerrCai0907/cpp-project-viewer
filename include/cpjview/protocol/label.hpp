#pragma once

#include <string>

namespace cpjview::protocol {

enum class LabelKind : uint32_t {
  Record,
};

/// @brief Label can force to create an orphan symbol even there are no
/// relationship.
struct Label {
  std::string m_symbol;
  LabelKind m_label;

  static Label from_json(std::string json_str);
  std::string to_json() const;
};

} // namespace cpjview::protocol
