#pragma once

#include "cpjview/server/persistence/inheritance.hpp"
#include <map>
#include <set>
#include <vector>

namespace cpjview::persistence {

class Project {
  struct Element {};
  using Map =

      std::map<const char *, class> const char *m_name{};
  Inheritance m_inheritance{};

public:
};

} // namespace cpjview::persistence
