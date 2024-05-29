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

  void put_code(std::string const &project_name, std::string const &name,
                std::string const &code);

  Result<const char *, ErrorCode> get_code(std::string const &project_name,
                                           std::string const &name) noexcept;

private:
  StorageImpl *m_impl;
};

} // namespace cpjview::persistence
