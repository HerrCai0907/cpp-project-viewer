#pragma once

#include "cpjview/common/result.hpp"
#include "spdlog/spdlog.h"
#include "sqlite3.h"
#include <cassert>
#include <memory>
#include <string>

namespace cpjview::persistence::sqlite3 {

namespace detail {

struct Sqlite3Deleter {
  void operator()(struct sqlite3 *ptr) {
    for (;;) {
      int ret = sqlite3_close(ptr);
      if (ret == SQLITE_OK) {
        break;
      }
      if (ret == SQLITE_BUSY) {
        spdlog::warn("[sqlite] close sqlite with unfinished operation");
        continue;
      }
      spdlog::error("[sqlite] sqlite error {} when closing",
                    sqlite3_errmsg(ptr));
    }
  }
};

using Sqlite3DbUniquePtr = std::unique_ptr<struct sqlite3, Sqlite3Deleter>;

struct TableDeleter {
  void operator()(struct sqlite3 *ptr) {}
};

} // namespace detail

class Sqlite3 {
  detail::Sqlite3DbUniquePtr m_db;

public:
  Sqlite3() {}

  Result<void, const char *> connect();

  void exec(std::string const &command);
};

} // namespace cpjview::persistence::sqlite3
