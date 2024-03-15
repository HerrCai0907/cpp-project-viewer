#include "cpjview/server/server.hpp"
#include "cpjview/persistence/persistence.hpp"
#include "cpjview/utils/result.hpp"
#include "spdlog/spdlog.h"
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <thread>

namespace cpjview::server {

class ServerImpl {
  httplib::Server m_server;
  std::thread m_listen_thread;
  persistence::Storage const &m_storage;

public:
  explicit ServerImpl(persistence::Storage const &storage)
      : m_server(), m_listen_thread(), m_storage{storage} {}

  ~ServerImpl() {
    if (m_listen_thread.joinable()) {
      m_listen_thread.join();
    }
  }

  Result<void, Server::Error> start(std::string const &resource,
                                    std::string const &host, int port) {
    {
      Result<void, Server::Error> ret = mount(resource);
      if (ret.nok()) {
        spdlog::error("fail mounting {} to '/'", resource);
        return ret;
      }
      spdlog::debug("success mounting {} to '/'", resource);
    }
    register_apis();
    {
      Result<void, Server::Error> ret = listen(host, port);
      if (ret.nok()) {
        spdlog::error("fail listening {}:{}", host, port);
        return ret;
      }
      spdlog::debug("success listening http://{}:{}", host, port);
    }
    spdlog::info("backend server enabled");
    return Result<void, Server::Error>::success();
  }

private:
  Result<void, Server::Error> listen(const std::string &host, int port) {
    if (!m_server.bind_to_port(host, port)) {
      return Result<void, Server::Error>::failed(Server::Error::cannot_bind);
    }
    m_listen_thread = std::thread([this]() { m_server.listen_after_bind(); });
    return Result<void, Server::Error>::success();
  }

  Result<void, Server::Error> mount(const std::string &mount_path) {
    if (!m_server.set_mount_point("/", mount_path)) {
      return Result<void, Server::Error>::failed(
          Server::Error::cannot_mount_root_dir);
    }
    return Result<void, Server::Error>::success();
  }

  void register_apis() { register_dependence_graph(); }

  void register_dependence_graph() {
    m_server.Get(
        "/api/dependence_graph",
        [this](const httplib::Request &, httplib::Response &response) noexcept {
          spdlog::trace("process \"/api/dependence_graph\"");
          std::vector<persistence::Storage::InheritancePair>
              inheritance_relationships =
                  m_storage.get_inheritance_relationships();
          nlohmann::json content = nlohmann::json::array();
          for (persistence::Storage::InheritancePair const &relationship :
               inheritance_relationships) {
            nlohmann::json json = nlohmann::json::object();
            json["base"] = relationship.base;
            json["derived"] = relationship.derived;
            content.push_back(std::move(json));
          }
          spdlog::trace("response {}", content.dump());
          response.set_content(content.dump(), "application/json");
        });
  }
};

Server::Server(persistence::Storage const &storage) {
  m_impl = new ServerImpl(storage);
}

Server::~Server() { delete m_impl; }

Result<void, Server::Error> Server::start(std::string const &resource,
                                          std::string const &host, int port) {
  return m_impl->start(resource, host, port);
}

}; // namespace cpjview::server
