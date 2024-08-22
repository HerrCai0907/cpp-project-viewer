#pragma once

#include "cpjview/common/result.hpp"
#include <cassert>
#include <memory>
#include <string>

struct sqlite3;

namespace cpjview::persistence::sqlite3 {

class Sqlite3 {
public:
  struct DbDeleter {
    void operator()(struct sqlite3 *ptr) const;
  };
  using DbUniquePtr = std::unique_ptr<struct sqlite3, DbDeleter>;

  struct Response {
    char **m_message;
    int m_row;
    int m_column;
  };
  struct ResponseDeleter {
    void operator()(Response *response) const;
  };
  using ResponseUniquePtr = std::unique_ptr<Response, ResponseDeleter>;

private:
  DbUniquePtr m_db;

public:
  Sqlite3() {}

  Result<void, const char *> connect();

  ResponseUniquePtr exec(std::string const &command);
};

} // namespace cpjview::persistence::sqlite3
