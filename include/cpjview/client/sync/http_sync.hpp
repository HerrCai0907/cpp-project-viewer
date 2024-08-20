#pragma once

#include "cpjview/client/sync/sync.hpp"
#include "cpjview/protocol/label.hpp"
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

  void add_source_code_relationship(std::string const &name,
                                    std::string const &code) override;

  void mark_label(std::string const &symbol,
                  protocol::LabelKind label) override;

private:
  void execute_http_patch(std::string const &uri, std::string const &body);
};

} // namespace cpjview::sync
