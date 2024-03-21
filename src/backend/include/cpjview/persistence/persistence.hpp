#pragma once

#include <string_view>
#include <vector>

namespace cpjview::persistence {

class StorageImpl;

class Storage {
public:
  Storage();
  ~Storage();

  void add_inheritance_relationship(std::string_view derived,
                                    std::string_view base);

  struct InheritancePair {
    const char *derived;
    const char *base;
    bool operator==(InheritancePair const &o) const {
      return std::string_view{derived} == std::string_view{o.derived} &&
             std::string_view{base} == std::string_view{o.base};
    }
    bool operator!=(InheritancePair const &o) const { return !((*this) == o); }
  };
  std::vector<InheritancePair> get_inheritance_relationships() const;

private:
  StorageImpl *m_impl;
};

} // namespace cpjview::persistence
