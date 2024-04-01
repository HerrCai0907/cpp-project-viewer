#pragma once

#include "cpjview/utils/result.hpp"
#include <string>
#include <string_view>
#include <system_error>

namespace cpjview {

[[nodiscard]] Result<std::string, std::error_code>
normalize_path(std::string_view path);

} // namespace cpjview
