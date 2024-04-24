#pragma once

#include <string>

namespace cpjview::sync {

class ISync {
public:
  virtual void add_inheritance_relationship(std::string const &derived,
                                            std::string const &base) = 0;
};

} // namespace cpjview::sync
