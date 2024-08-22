
#include "sqlite3.hpp"
#include "spdlog/spdlog.h"
#include "sqlite3.h"
#include <memory>

namespace cpjview::persistence::sqlite3 {

void Sqlite3::DbDeleter::operator()(struct sqlite3 *ptr) const {
  for (;;) {
    int ret = sqlite3_close(ptr);
    if (ret == SQLITE_OK) {
      break;
    }
    if (ret == SQLITE_BUSY) {
      spdlog::warn("[sqlite] close sqlite with unfinished operation");
      continue;
    }
    spdlog::error("[sqlite] sqlite error {} when closing", sqlite3_errmsg(ptr));
  }
}

void Sqlite3::ResponseDeleter::operator()(Sqlite3::Response *response) const {
  sqlite3_free_table(response->m_message);
}

Result<void, const char *> Sqlite3::connect() {
  using RT = Result<void, const char *>;
  struct sqlite3 *pDb;
  int ret = sqlite3_open("sqlite3.db", &pDb);
  if (pDb == nullptr) {
    spdlog::error("[sqlite] failed to open due to OOM");
    return RT::failed("OOM");
  }
  m_db.reset(pDb);
  if (ret != SQLITE_OK) {
    const char *msg = sqlite3_errmsg(pDb);
    spdlog::error("[sqlite] failed to open due to {}", msg);
    return RT::failed(msg);
  }
  spdlog::trace("[sqlite] success to open in sqlite3.db");
  return RT::success();
}

Sqlite3::ResponseUniquePtr Sqlite3::exec(std::string const &command) {
  char *err_msg;
  ResponseUniquePtr ret{new Response{}};
  int ret_code = sqlite3_get_table(m_db.get(), command.c_str(), &ret->m_message,
                                   &ret->m_row, &ret->m_column, &err_msg);
  if (ret_code != SQLITE_OK) {
    spdlog::trace("[sqlite] exec {} failed, error msg {}", command, err_msg);
    return nullptr;
  }
  for (int i = 0; i < ret->m_row; i++) {
    for (int j = 0; j < ret->m_column; i++) {
      spdlog::trace("[sqlite] result[{},{}]: {}", i, j,
                    ret->m_message[i * ret->m_column + j]);
    }
  }
  return ret;
}

} // namespace cpjview::persistence::sqlite3
