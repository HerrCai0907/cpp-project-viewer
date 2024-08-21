
#include "sqlite3.hpp"
#include "spdlog/spdlog.h"

namespace cpjview::persistence::sqlite3 {

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

void Sqlite3::exec(std::string const &command) {
  char **az_result;
  int n_row = 0;
  int n_column = 0;
  char *z_err_msg;
  int ret = sqlite3_get_table(m_db.get(), command.c_str(), &az_result, &n_row,
                              &n_column, &z_err_msg);

  if (ret != SQLITE_OK) {
    spdlog::trace("[sqlite] z_err_msg {}", z_err_msg);
    return;
  }
  spdlog::trace("[sqlite] row {} column {}", n_row, n_column);
  for (int i = 0; i < n_column * n_row; i++) {
    spdlog::trace("[sqlite] az_result[{}] {}", i, az_result[i]);
  }

  sqlite3_free_table(az_result);
}

} // namespace cpjview::persistence::sqlite3
