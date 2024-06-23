#pragma once

#include <string>

namespace cpjview::sync {

class ISync {
public:
  virtual void add_inheritance_relationship(std::string const &derived,
                                            std::string const &base) = 0;

  virtual void add_code(std::string const &name, std::string const &code) = 0;
};

} // namespace cpjview::sync
