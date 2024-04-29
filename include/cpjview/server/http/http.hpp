
#include "cpjview/common/result.hpp"
#include "cpjview/server/persistence/storage.hpp"

namespace cpjview::http {

class HttpImpl;

class Http {
  HttpImpl *m_impl;

public:
  enum class Error {
    cannot_mount_root_dir,
    cannot_bind,
  };

  explicit Http(persistence::Storage &storage);
  ~Http();

  Result<void, Error> start(std::string const &resource,
                            std::string const &host, int port);
};

std::string to_string(Http::Error err);

} // namespace cpjview::http
