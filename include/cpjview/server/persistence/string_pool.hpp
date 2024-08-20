#pragma once

#include <set>
#include <string>

namespace cpjview::persistence {

class StringPool {
  using StringCacheType = std::set<std::string>;
  StringCacheType m_string_cache{};

public:
  using StringIndex = const char *; // FIXME

  const char *ensure_string_in_cache(std::string_view str) {
    StringCacheType::const_iterator it =
        m_string_cache.insert(std::string{str}).first;
    return it->c_str();
  }
};

} // namespace cpjview::persistence