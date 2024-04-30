#pragma once

#include "cpjview/server/persistence/error_code.hpp"
#include "cpjview/server/persistence/inheritance.hpp"
#include "cpjview/server/persistence/kv.hpp"

namespace cpjview::persistence {

class Project {
public:
  struct Info {
    Inheritance m_inheritance{};
  };

  using Iterator = SearchMap<Info>::Iterator;
  using ConstIterator = SearchMap<Info>::ConstIterator;

private:
  SearchMap<Info> m_search_map;

public:
  ErrorCodeResult create(const char *name) {
    if (m_search_map.contains(name)) {
      return ErrorCodeResult::failed(ErrorCode::forbidden());
    }
    m_search_map.renew(name);
    return ErrorCodeResult::success();
  }

  ErrorCodeResult force_create(const char *name) {
    m_search_map.renew(name);
    return ErrorCodeResult::success();
  }

  ErrorCodeResult
  modify_info(const char *key,
              std::function<ErrorCodeResult(Info &)> const &fn) {
    return m_search_map.modify(key, fn);
  }

  Info *get_info(const char *key) { return m_search_map.get(key); }

  Iterator begin() { return m_search_map.begin(); }
  Iterator end() { return m_search_map.end(); }
  ConstIterator begin() const { return m_search_map.begin(); }
  ConstIterator end() const { return m_search_map.end(); }
};

} // namespace cpjview::persistence
