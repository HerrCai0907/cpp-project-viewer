#pragma once

#include "cpjview/common/result.hpp"
#include "cpjview/server/persistence/error_code.hpp"
#include "cpjview/server/persistence/kv.hpp"
#include <string>
#include <vector>

namespace cpjview::persistence {

class StorageImpl;

class Storage {
public:
  Storage();
  ~Storage();

  ErrorCodeResult put_project(std::string const &name);

  std::vector<const char *> get_projects() const;

  void add_inheritance(std::string const &project_name,
                       std::string const &derived, std::string const &base);

  struct InheritancePair {
    const char *derived;
    const char *base;
  };
  Result<std::vector<InheritancePair>, ErrorCode>
  get_all_inheritance(std::string const &project_name) const;

private:
  StorageImpl *m_impl;
};

} // namespace cpjview::persistence
