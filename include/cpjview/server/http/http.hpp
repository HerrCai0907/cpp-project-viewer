
#include "cpjview/common/result.hpp"
#include "cpjview/server/persistence/storage.hpp"

namespace cpjview::http {

class HttpServerImpl;

class HttpServer {
  HttpServerImpl *m_impl;

public:
  enum class Error {
    cannot_mount_root_dir,
    cannot_bind,
  };

  explicit HttpServer(persistence::Storage &storage);
  ~HttpServer();

  Result<void, Error> start(std::string const &resource,
                            std::string const &host, int port);
};

std::string to_string(HttpServer::Error err);

} // namespace cpjview::http
