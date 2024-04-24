#include "cpjview/common/argparser.hpp"
#include "spdlog/common.h"
#include <cassert>
#include <span>
#include <spdlog/spdlog.h>
#include <string>

namespace cpjview {

void ArgParser::add_logger_option() {
  bool success = true;
  success = m_options
                .insert_or_assign("--debug",
                                  [](std::span<const char *> &argv) {
                                    spdlog::set_level(spdlog::level::debug);
                                    spdlog::debug("enable debug log");
                                  })
                .second;
  assert(success);
  success = m_options
                .insert_or_assign("--verbose",
                                  [](std::span<const char *> &argv) {
                                    spdlog::set_level(spdlog::level::trace);
                                    spdlog::debug("enable verbose log");
                                  })
                .second;
  assert(success);
  static_cast<void>(success);
}

void ArgParser::add_option(uint32_t &option_value, const char *name,
                           const char *desc) {
  bool success = m_options
                     .insert_or_assign(
                         name,
                         [this, &option_value](std::span<const char *> &argv) {
                           option_value = std::stoul(argv[0]);
                           argv = argv.subspan(1);
                         })
                     .second;

  assert(success);
}

void ArgParser::add_option(std::string &option_value, const char *name,
                           const char *desc) {
  bool success = m_options
                     .insert_or_assign(
                         name,
                         [this, &option_value](std::span<const char *> &argv) {
                           option_value = argv[0];
                           argv = argv.subspan(1);
                         })
                     .second;

  assert(success);
}

void ArgParser::add_option(std::vector<std::string> &option_value,
                           const char *name, const char *desc) {
  bool success = m_options
                     .insert_or_assign(
                         name,
                         [this, &option_value](std::span<const char *> &argv) {
                           option_value.push_back(argv[0]);
                           argv = argv.subspan(1);
                         })
                     .second;

  assert(success);
}

Result<void, std::string> ArgParser::parse(std::span<const char *> argv) {
  argv = argv.subspan(1); // ignore current process name
  while (!argv.empty()) {
    auto it = m_options.find(argv[0]);
    if (it != m_options.end()) {
      argv = argv.subspan(1);
      it->second(argv);
      continue;
    }
    return Result<void, std::string>::failed(
        std::string{"unknown command option: "} + argv[0]);
  }
  return Result<void, std::string>::success();
}

} // namespace cpjview