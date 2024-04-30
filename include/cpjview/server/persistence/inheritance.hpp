#pragma once

#include "cpjview/server/persistence/kv.hpp"
#include "cpjview/server/persistence/storage.hpp"
#include <set>
#include <vector>

namespace cpjview::persistence {

class Inheritance {
  struct Info {
    std::set<const char *> m_derived{};
    std::set<const char *> m_base{};
  };

  SearchMap<Info> m_search_map;

public:
  void add_inheritance(const char *derived, const char *base);

  std::vector<Storage::InheritancePair> get_all_inheritance() const;
};

} // namespace cpjview::persistence
