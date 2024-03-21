#include "cpjview/analysis/analysis.hpp"
#include "cpjview/analysis/inheritance.hpp"
#include "cpjview/loader/filter.hpp"
#include "cpjview/loader/loader.hpp"
#include "cpjview/server/server.hpp"
#include "cpjview/utils/argparser.hpp"
#include "spdlog/spdlog.h"
#include <cstdlib>

using namespace cpjview;

int main(int argc, const char *argv[]) {
  spdlog::flush_on(spdlog::level::trace);

  ArgParser arg_parser{};
  std::string compilation_database_path{};
  arg_parser.add_option(compilation_database_path, "-p", "FIXME");

  std::vector<std::string> include_paths{};
  arg_parser.add_option(include_paths, "--include", "FIXME");

  enum class DebugMode { None, Debug, Verbose };
  DebugMode debug_mode = DebugMode::None;
  arg_parser.add_option<DebugMode>(
      debug_mode,
      {{"--debug", DebugMode::Debug}, {"--verbose", DebugMode::Verbose}},
      "FIXME");

  Result<void, std::string> parse_ret = arg_parser.parse(
      llvm::ArrayRef<const char *>{argv, static_cast<size_t>(argc)});
  if (parse_ret.nok()) {
    spdlog::error(parse_ret.take_error());
    std::exit(1);
  }

  switch (debug_mode) {
  case DebugMode::None:
    break;
  case DebugMode::Debug:
    spdlog::set_level(spdlog::level::debug);
    spdlog::debug("enable debug log");
    break;
  case DebugMode::Verbose:
    spdlog::set_level(spdlog::level::trace);
    spdlog::debug("enable trace log");
    break;
  }

  persistence::Storage storage{};

  server::Server server{storage};
  server.start("src/www", "127.0.0.1", 8100);

  loader::Filter filter{include_paths};

  loader::Loader loader{};
  Result<void, std::string> load_ret =
      loader.load(compilation_database_path, filter);
  if (load_ret.nok()) {
    spdlog::error(load_ret.take_error());
    std::exit(1);
  }
  std::vector<std::unique_ptr<clang::ASTUnit>> asts = loader.create_ast();

  for (std::unique_ptr<clang::ASTUnit> const &ast : asts) {
    analysis::Context context{
        .m_ast_unit = ast.get(), .m_filter = &filter, .m_storage = &storage};
    analysis::InheritanceAnalysis{context}.start();
  }
}
