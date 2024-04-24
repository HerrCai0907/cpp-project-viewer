#pragma once

#include <string>
#include <vector>

namespace cpjview::persistence {

class StorageImpl;

class Storage {
public:
  Storage();
  ~Storage();

  void add_inheritance_relationship(std::string const &derived,
                                    std::string const &base);

  struct InheritancePair {
    const char *derived;
    const char *base;
  };
  std::vector<InheritancePair> get_inheritance_relationships() const;

private:
  StorageImpl *m_impl;
};

} // namespace cpjview::persistence
