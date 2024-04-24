#include "cpjview/common/argparser.hpp"
#include "cpjview/server/http/http.hpp"
#include "cpjview/server/persistence/persistence.hpp"
#include <cstdint>
#include <span>
#include <spdlog/spdlog.h>

using namespace cpjview;

struct Option {
  std::uint32_t port{};
};

static Option parse_args(std::span<const char *> args) {
  Option option{};
  ArgParser arg_parser{};

  arg_parser.add_logger_option();

  arg_parser.add_option(option.port, "--port", "port");

  Result<void, std::string> parse_ret = arg_parser.parse(args);
  if (parse_ret.nok()) {
    spdlog::error(parse_ret.take_error());
    std::exit(1);
  }

  return option;
}

int main(int argc, const char *argv[]) {
  Option option =
      parse_args(std::span<const char *>{argv, static_cast<size_t>(argc)});

  persistence::Storage storage{};

  http::Http http_server{storage};
  Result<void, http::Http::Error> ret =
      http_server.start("src/www", "127.0.0.1", 8000);
  if (ret.nok()) {
    spdlog::error(http::to_string(ret.take_error()));
    std::exit(1);
  }
}
