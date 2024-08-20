#pragma once

#include "cpjview/protocol/label.hpp"
#include <string>

namespace cpjview::sync {

class ISync {
public:
  virtual void add_inheritance_relationship(std::string const &derived,
                                            std::string const &base) = 0;

  virtual void add_source_code_relationship(std::string const &name,
                                            std::string const &code) = 0;

  virtual void mark_label(std::string const &symbol,
                          protocol::LabelKind label) = 0;
};

} // namespace cpjview::sync
