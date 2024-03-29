#include "cpjview/loader/filter.hpp"
#include "cpjview/utils/result.hpp"
#include "spdlog/spdlog.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include <cassert>
#include <system_error>

namespace cpjview::loader {

[[nodiscard]] static Result<std::string, std::error_code>
normalize_path(llvm::StringRef path) {
  llvm::SmallString<64> absolute_path{path};
  if (std::error_code error = llvm::sys::fs::make_absolute(absolute_path)) {
    return Result<std::string, std::error_code>::failed(error);
  }
  llvm::sys::path::remove_dots(absolute_path, true);
  return Result<std::string, std::error_code>::success(
      std::string{absolute_path});
}

[[nodiscard]] static bool
is_file_included(std::string const &absolute_file_path,
                 std::vector<std::string> const &absolute_include_paths) {
  for (std::string const &include_path : absolute_include_paths) {
    if (absolute_file_path.size() >= include_path.size() &&
        llvm::StringRef{absolute_file_path.data(), include_path.size()} ==
            include_path) {
      return true;
    }
  }
  return false;
}

Filter::Filter(llvm::ArrayRef<std::string> include_paths)
    : m_include_paths{}, m_cache{} {
  for (std::string const &include_path : include_paths) {
    auto normalize_path_result = normalize_path(include_path);
    if (normalize_path_result.nok()) {
      spdlog::error("ignore include_paths {}, {}", include_path,
                    normalize_path_result.take_error().message());
    }
    m_include_paths.push_back(normalize_path_result.get());
  }
}

bool Filter::validate(std::string_view file_path,
                      std::string *normalized_file_path) const {
  assert(!file_path.empty());
  auto normalize_path_result = normalize_path(file_path);
  if (normalize_path_result.nok()) {
    spdlog::error("ignore file {}, {}", file_path,
                  normalize_path_result.take_error().message());
    return false;
  }
  if (normalized_file_path) {
    *normalized_file_path = normalize_path_result.get();
  }
  return validate_impl(normalize_path_result.get());
}

bool Filter::validate_impl(std::string const &normalized_file_path) const {
  auto it = m_cache.find(normalized_file_path);
  if (it != m_cache.end()) {
    return it->second;
  }
  bool is_include = is_file_included(normalized_file_path, m_include_paths);
  m_cache.insert_or_assign(normalized_file_path, is_include);
  return is_include;
}

} // namespace cpjview::loader
