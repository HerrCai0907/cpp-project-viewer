#pragma once

#include "result.hpp"
#include <cassert>
#include <functional>
#include <map>
#include <span>
#include <string>
#include <vector>

namespace cpjview {

class ArgParser {
public:
  void add_logger_option();

  void add_option(uint32_t &option_value, const char *name, const char *desc);

  void add_option(std::string &option_value, const char *name,
                  const char *desc);

  template <class Enum>
  void add_option(Enum &option_value,
                  std::map<const char *, Enum> const &name_enum_map,
                  const char *desc) {
    for (std::pair<const char *, Enum> const &v : name_enum_map) {
      const char *enum_name = v.first;
      Enum enum_value = v.second;
      bool success = m_options
                         .insert_or_assign(enum_name,
                                           [this, enum_value, &option_value](
                                               std::span<const char *> &) {
                                             option_value = enum_value;
                                           })
                         .second;
      assert(success);
    }
  }

  void add_option(std::vector<std::string> &option_value, const char *name,
                  const char *desc);

  [[nodiscard]] Result<void, std::string> parse(std::span<const char *> argv);

private:
  std::map<std::string_view, std::function<void(std::span<const char *> &argv)>>
      m_options;
};

} // namespace cpjview
