
#include "cpjview/persistence/persistence.hpp"
#include "cpjview/utils/result.hpp"

namespace cpjview::server {

class ServerImpl;

class Server {
  ServerImpl *m_impl;

public:
  enum class Error {
    cannot_mount_root_dir,
    cannot_bind,
  };

  explicit Server(persistence::Storage const &storage);
  ~Server();

  Result<void, Error> start(std::string const &resource,
                            std::string const &host, int port);
};

} // namespace cpjview::server
