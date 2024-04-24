#pragma once

#include "cpjview/client/sync/sync.hpp"
#include <httplib.h>
#include <string>

namespace cpjview::sync {

class HttpSync : public ISync {
  std::string m_project_name;
  httplib::Client m_client;

public:
  explicit HttpSync(std::string project_name);

  void add_inheritance_relationship(std::string const &derived,
                                    std::string const &base) override;
};

} // namespace cpjview::sync
