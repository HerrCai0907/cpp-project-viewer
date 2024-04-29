#include "cpjview/server/http/http.hpp"
#include "cpjview/common/result.hpp"
#include "cpjview/protocol/inheritance.hpp"
#include "cpjview/server/persistence/storage.hpp"
#include <filesystem>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <thread>

namespace cpjview::http {

class HttpImpl {
  httplib::Server m_server;
  std::thread m_listen_thread;
  persistence::Storage &m_storage;

public:
  explicit HttpImpl(persistence::Storage &storage)
      : m_server(), m_listen_thread(), m_storage{storage} {}

  ~HttpImpl() {
    if (m_listen_thread.joinable()) {
      m_listen_thread.join();
    }
  }

  Result<void, Http::Error> start(std::string const &resource,
                                  std::string const &host, int port) {
    {
      Result<void, Http::Error> ret = mount(resource);
      if (ret.nok()) {
        spdlog::error("fail mounting {} to '/'", resource);
        return ret;
      }
      spdlog::debug("success mounting {} to '/'", resource);
    }
    register_apis();
    {
      Result<void, Http::Error> ret = listen(host, port);
      if (ret.nok()) {
        spdlog::error("fail listening {}:{}", host, port);
        return ret;
      }
      spdlog::debug("success listening http://{}:{}", host, port);
    }
    spdlog::info("backend server enabled in http://{}:{}", host, port);
    return Result<void, Http::Error>::success();
  }

private:
  Result<void, Http::Error> listen(const std::string &host, int port) {
    if (!m_server.bind_to_port(host, port)) {
      return Result<void, Http::Error>::failed(Http::Error::cannot_bind);
    }
    m_listen_thread = std::thread([this]() { m_server.listen_after_bind(); });
    return Result<void, Http::Error>::success();
  }

  Result<void, Http::Error> mount(const std::string &mount_path) {
    std::filesystem::path absolute_mount_path =
        std::filesystem::absolute(mount_path);
    spdlog::trace("[http] mount {} to '/'", mount_path);
    if (!m_server.set_mount_point("/", absolute_mount_path.string())) {
      return Result<void, Http::Error>::failed(
          Http::Error::cannot_mount_root_dir);
    }
    return Result<void, Http::Error>::success();
  }

  void register_apis() {
    register_project();
    register_inheritance();
  }

  void register_project() {
    m_server.Get("/api/v1/projects",
                 [this](const httplib::Request &, httplib::Response &response) {
                   response.set_content("", "application/json");
                 });
  }

  void register_inheritance() {
    m_server.Patch(
        "/api/v1/projects/:project/inheritance",
        [this](const httplib::Request &request, httplib::Response &response) {
          spdlog::trace("[http] process path {} with body {}", request.path,
                        request.body);
          std::string const &project = request.path_params.at("project");
          protocol::Inheritance inheritance =
              protocol::Inheritance::from_json(request.body);
          m_storage.add_inheritance(inheritance.m_derived, inheritance.m_base);
        });
    m_server.Get(
        "/inheritance_graph",
        [this](const httplib::Request &, httplib::Response &response) noexcept {
          spdlog::trace("[http] process \"/inheritance_graph\"");
          std::vector<persistence::Storage::InheritancePair>
              inheritance_relationships = m_storage.get_all_inheritance();
          nlohmann::json content = nlohmann::json::array();
          for (persistence::Storage::InheritancePair const &relationship :
               inheritance_relationships) {
            const nlohmann::json json = {
                {"base", relationship.base},
                {"derived", relationship.derived},
            };
            content.push_back(std::move(json));
          }
          spdlog::trace("[http] response {}", content.dump());
          response.set_content(content.dump(), "application/json");
        });
  }
};

Http::Http(persistence::Storage &storage) { m_impl = new HttpImpl(storage); }

Http::~Http() { delete m_impl; }

Result<void, Http::Error> Http::start(std::string const &resource,
                                      std::string const &host, int port) {
  return m_impl->start(resource, host, port);
}

std::string to_string(Http::Error err) {
  switch (err) {
  case Http::Error::cannot_mount_root_dir:
    return "cannot mount resource folder";
  case Http::Error::cannot_bind:
    return "cannot bind to port";
  }
  return "unknown";
}

}; // namespace cpjview::http
