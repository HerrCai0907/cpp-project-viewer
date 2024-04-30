#pragma once

#include "cpjview/common/result.hpp"
#include "cpjview/server/persistence/error_code.hpp"
#include <map>

namespace cpjview::persistence {

using ErrorCodeResult = Result<void, ErrorCode>;

template <class E> class SearchMap {
public:
  using Map = std::map<const char *, E>;
  using Iterator = typename Map::iterator;
  using ConstIterator = typename Map::const_iterator;

private:
  Map m_map{};

public:
  bool contains(const char *key) { return m_map.contains(key); }
  E *get(const char *key) {
    Iterator it = m_map.find(key);
    return it == m_map.end() ? nullptr : &it->second;
  }

  Iterator renew(const char *key) {
    return m_map.insert_or_assign(key, E{}).first;
  }

  ErrorCodeResult modify(const char *key,
                         std::function<ErrorCodeResult(E &)> const &fn) {
    Iterator it = m_map.find(key);
    if (it == m_map.end()) {
      it = renew(key);
    }
    return fn(it->second);
  }

  Iterator begin() { return m_map.begin(); }
  Iterator end() { return m_map.end(); }
  ConstIterator begin() const { return m_map.begin(); }
  ConstIterator end() const { return m_map.end(); }
};

} // namespace cpjview::persistence