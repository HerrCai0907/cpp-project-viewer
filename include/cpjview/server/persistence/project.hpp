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
  void ensure_class();

  void ensure_inheritance();
};

} // namespace cpjview::persistence
