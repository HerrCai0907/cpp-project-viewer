#include "cpjview/server/http/http.hpp"
#include "cpjview/common/result.hpp"
#include "cpjview/protocol/inheritance.hpp"
#include "cpjview/server/persistence/error_code.hpp"
#include "cpjview/server/persistence/storage.hpp"
#include "nlohmann/json_fwd.hpp"
#include <filesystem>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <sstream>
#include <thread>

namespace cpjview::http {

static std::string to_string(const httplib::Request &req) {
  std::stringstream ss{};
  ss << req.method << " " << req.path;
  if (!req.body.empty()) {
    ss << " with body " << req.body;
  }
  return ss.str();
}

class HttpServerImpl {
  httplib::Server m_server;
  std::thread m_listen_thread;
  persistence::Storage &m_storage;

public:
  explicit HttpServerImpl(persistence::Storage &storage)
      : m_server(), m_listen_thread(), m_storage{storage} {}

  ~HttpServerImpl() {
    if (m_listen_thread.joinable()) {
      m_listen_thread.join();
    }
  }

  Result<void, HttpServer::Error> start(std::string const &resource,
                                        std::string const &host, int port) {
    {
      Result<void, HttpServer::Error> ret = mount(resource);
      if (ret.nok()) {
        spdlog::error("fail mounting {} to '/'", resource);
        return ret;
      }
      spdlog::debug("success mounting {} to '/'", resource);
    }
    register_apis();
    {
      Result<void, HttpServer::Error> ret = listen(host, port);
      if (ret.nok()) {
        spdlog::error("fail listening {}:{}", host, port);
        return ret;
      }
      spdlog::debug("success listening http://{}:{}", host, port);
    }
    spdlog::info("backend server enabled in http://{}:{}", host, port);
    return Result<void, HttpServer::Error>::success();
  }

private:
  Result<void, HttpServer::Error> listen(const std::string &host, int port) {
    if (!m_server.bind_to_port(host, port)) {
      return Result<void, HttpServer::Error>::failed(
          HttpServer::Error::cannot_bind);
    }
    m_listen_thread = std::thread([this]() { m_server.listen_after_bind(); });
    return Result<void, HttpServer::Error>::success();
  }

  Result<void, HttpServer::Error> mount(const std::string &mount_path) {
    std::filesystem::path absolute_mount_path =
        std::filesystem::absolute(mount_path);
    spdlog::trace("[http] mount {} to '/'", mount_path);
    if (!m_server.set_mount_point("/", absolute_mount_path.string())) {
      return Result<void, HttpServer::Error>::failed(
          HttpServer::Error::cannot_mount_root_dir);
    }
    return Result<void, HttpServer::Error>::success();
  }

  void register_apis() {
    register_project();
    register_inheritance();
  }

  void register_project() {
    m_server.Get("/api/v1/projects", [this](const httplib::Request &,
                                            httplib::Response &response) {
      std::vector<const char *> project_list = m_storage.get_projects();
      nlohmann::json content_json = nlohmann::json::array();
      for (const char *project_name : project_list) {
        content_json.push_back(nlohmann::json{
            {"name", std::string{project_name}},
        });
      }
      const std::string content = content_json.dump();
      spdlog::trace("[http] response {}", content);
      response.set_content(content, "application/json");
    });
  }

  void register_inheritance() {
    constexpr const char *path_pattern =
        "/api/v1/projects/:project/inheritances";
    m_server.Patch(path_pattern, [this](const httplib::Request &request,
                                        httplib::Response &response) {
      spdlog::trace("[http] recv {}", to_string(request));
      std::string const &project = request.path_params.at("project");
      protocol::Inheritance inheritance =
          protocol::Inheritance::from_json(request.body);
      m_storage.add_inheritance(project, inheritance.m_derived,
                                inheritance.m_base);
    });
    m_server.Get(path_pattern, [this](const httplib::Request &request,
                                      httplib::Response &response) {
      spdlog::trace("[http] recv {}", to_string(request));
      std::string const &project = request.path_params.at("project");
      Result<std::vector<persistence::Storage::InheritancePair>,
             persistence::ErrorCode>
          inheritances = m_storage.get_all_inheritance(project);
      if (inheritances.nok()) {
        response.status = static_cast<int>(inheritances.take_error().m_code);
        return;
      }
      nlohmann::json content_json = nlohmann::json::array();
      for (persistence::Storage::InheritancePair const &relationship :
           inheritances.get()) {
        content_json.push_back(nlohmann::json{
            {"base", relationship.base},
            {"derived", relationship.derived},
        });
      }
      const std::string content = content_json.dump();
      spdlog::trace("[http] response {}", content);
      response.set_content(content, "application/json");
    });
  }
};

HttpServer::HttpServer(persistence::Storage &storage) {
  m_impl = new HttpServerImpl(storage);
}

HttpServer::~HttpServer() { delete m_impl; }

Result<void, HttpServer::Error> HttpServer::start(std::string const &resource,
                                                  std::string const &host,
                                                  int port) {
  return m_impl->start(resource, host, port);
}

std::string to_string(HttpServer::Error err) {
  switch (err) {
  case HttpServer::Error::cannot_mount_root_dir:
    return "cannot mount resource folder";
  case HttpServer::Error::cannot_bind:
    return "cannot bind to port";
  }
  return "unknown";
}

}; // namespace cpjview::http
