#pragma once

#include "cpjview/common/result.hpp"
#include "cpjview/server/persistence/error_code.hpp"
#include "cpjview/server/persistence/string_pool.hpp"
#include <map>

namespace cpjview::persistence {

using ErrorCodeResult = Result<void, ErrorCode>;

template <class E> class SearchMap {
public:
  using Map = std::map<StringPool::StringIndex, E>;
  using Iterator = typename Map::iterator;
  using ConstIterator = typename Map::const_iterator;

private:
  Map m_map{};

public:
  bool contains(StringPool::StringIndex key) { return m_map.contains(key); }
  E *get(StringPool::StringIndex key) {
    Iterator it = m_map.find(key);
    return it == m_map.end() ? nullptr : &it->second;
  }
  Iterator renew(StringPool::StringIndex key) {
    return m_map.emplace(key, E{}).first;
  }
  Iterator try_add(StringPool::StringIndex key) {
    return m_map.try_emplace(key, E{}).first;
  }

  Iterator begin() { return m_map.begin(); }
  Iterator end() { return m_map.end(); }
  ConstIterator begin() const { return m_map.begin(); }
  ConstIterator end() const { return m_map.end(); }
};

} // namespace cpjview::persistence