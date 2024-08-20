#include "cpjview/client/sync/http_sync.hpp"
#include "cpjview/protocol/code.hpp"
#include "cpjview/protocol/inheritance.hpp"
#include "cpjview/protocol/label.hpp"
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

void HttpSync::execute_http_patch(std::string const &uri,
                                  std::string const &body) {
  spdlog::debug("PATCH {} with {}", uri, body);
  httplib::Result ret = m_client.Patch(uri, body, "application/json");
  if (!ret) {
    spdlog::error("cannot connect with server: {}",
                  httplib::to_string(ret.error()));
    exit(-1);
  }
  spdlog::debug("http response {}", ret->reason);
}

void HttpSync::add_inheritance_relationship(std::string const &derived,
                                            std::string const &base) {
  spdlog::debug("add_inheritance_relationship");
  const std::string uri =
      "/api/v1/projects/" + m_project_name + "/inheritances";
  const std::string body =
      protocol::Inheritance{.m_derived = derived, .m_base = base}.to_json();
  execute_http_patch(uri, body);
}

void HttpSync::add_source_code_relationship(std::string const &name,
                                            std::string const &code) {
  spdlog::debug("add_source_code_relationship");
  const std::string uri =
      "/api/v1/projects/" + m_project_name + "/source_codes";
  const std::string body =
      protocol::SourceCode{.m_name = name, .m_code = code}.to_json();
  execute_http_patch(uri, body);
}

void HttpSync::mark_label(std::string const &symbol,
                          protocol::LabelKind label) {
  spdlog::debug("mark_label");
  const std::string uri = "/api/v1/projects/" + m_project_name + "/labels";
  const std::string body =
      protocol::Label{.m_symbol = symbol, .m_label = label}.to_json();
  execute_http_patch(uri, body);
}

} // namespace cpjview::sync
