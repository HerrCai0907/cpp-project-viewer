#include "cpjview/client/utils/file_system.hpp"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"

namespace cpjview {

[[nodiscard]] Result<std::string, std::error_code>
normalize_path(std::string_view path) {
  llvm::SmallString<64> absolute_path{path};
  if (std::error_code error = llvm::sys::fs::make_absolute(absolute_path)) {
    return Result<std::string, std::error_code>::failed(error);
  }
  llvm::sys::path::remove_dots(absolute_path, true);
  return Result<std::string, std::error_code>::success(
      std::string{absolute_path});
}

} // namespace cpjview
