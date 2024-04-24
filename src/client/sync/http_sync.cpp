#include "cpjview/client/sync/http_sync.hpp"
#include "cpjview/protocol/inheritance.hpp"
#include "spdlog/spdlog.h"
#include <cstdlib>
#include <httplib.h>
#include <nlohmann/json.hpp>

namespace cpjview::sync {

HttpSync::HttpSync(std::string project_name)
    : m_project_name(std::move(project_name)), m_client{"127.0.0.1:8000"} {
  spdlog::info("{} http sync client start in {}", project_name,
               "127.0.0.1:8000");
}

void HttpSync::add_inheritance_relationship(std::string const &derived,
                                            std::string const &base) {
  spdlog::debug("add_inheritance_relationship");

  const std::string uri = "/api/v1/projects/" + m_project_name + "/inheritance";

  const std::string body =
      protocol::Inheritance{.m_derived = derived, .m_base = base}.to_json();

  spdlog::debug("PATCH {} with {}", uri, body);

  httplib::Result ret = m_client.Patch(uri, body, "application/json");

  if (!ret) {
    spdlog::error("cannot connect with server: {}",
                  httplib::to_string(ret.error()));
    exit(-1);
  }
  spdlog::debug("ret {}", ret->reason);
}

} // namespace cpjview::sync
