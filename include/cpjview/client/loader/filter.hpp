#pragma once

#include "llvm/ADT/ArrayRef.h"
#include <map>
#include <string>
#include <vector>

namespace cpjview::loader {

class Filter {
public:
  Filter(llvm::ArrayRef<std::string> include_paths);
  virtual ~Filter() = default;
  bool validate(std::string_view file_path) const {
    return validate(file_path, nullptr);
  }
  bool validate(std::string_view file_path,
                std::string *normalized_file_path) const;

private:
  std::vector<std::string> m_include_paths;
  mutable std::map<std::string, bool> m_cache;

  virtual bool validate_impl(std::string const &normalized_file_path) const;
};

} // namespace cpjview::loader
