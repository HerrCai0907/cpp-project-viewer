#pragma once

#include "cpjview/server/persistence/storage.hpp"
#include <map>
#include <set>
#include <vector>

namespace cpjview::persistence {

class Inheritance {
  struct Element {
    std::set<const char *> m_derived{};
    std::set<const char *> m_base{};
  };
  using Map = std::map<const char *, Element>;
  Map m_inheritance{};

public:
  void add_inheritance(const char *derived, const char *base);

  std::vector<Storage::InheritancePair> get_all_inheritance() const;
};

} // namespace cpjview::persistence
