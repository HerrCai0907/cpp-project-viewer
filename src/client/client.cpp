#include "cpjview/client/analysis/analysis.hpp"
#include "cpjview/client/analysis/inheritance.hpp"
#include "cpjview/client/loader/filter.hpp"
#include "cpjview/client/loader/loader.hpp"
#include "cpjview/client/sync/http_sync.hpp"
#include "cpjview/client/utils/task_priority.hpp"
#include "cpjview/common/argparser.hpp"
#include "cpjview/common/scheduler.hpp"
#include "spdlog/spdlog.h"
#include <cstdlib>

using namespace cpjview;

struct Option {
  std::string compilation_database_path{};
  std::vector<std::string> include_paths{};
  std::string project_name{};
};

static Option parse_args(std::span<const char *> args) {
  Option option{};

  ArgParser arg_parser{};

  arg_parser.add_logger_option();
  arg_parser.add_option(option.compilation_database_path, "-p",
                        "database path");

  arg_parser.add_option(option.include_paths, "--include", "filter");

  arg_parser.add_option(option.project_name, "--project-name", "project name");

  Result<void, std::string> parse_ret = arg_parser.parse(args);
  if (parse_ret.nok()) {
    spdlog::error(parse_ret.take_error());
    std::exit(1);
  }

  if (option.compilation_database_path.empty()) {
    spdlog::error("missing compilation database path, "
                  "use -p <compilation_database_path>");
    std::exit(1);
  }

  if (option.project_name.empty()) {
    spdlog::error("missing project name, "
                  "use --project-name <project_name>");
    std::exit(1);
  }

  return option;
}

int main(int argc, const char *argv[]) {
  Option option =
      parse_args(std::span<const char *>{argv, static_cast<size_t>(argc)});

  sync::HttpSync storage{option.project_name};

  loader::Filter filter{option.include_paths};

  loader::Loader loader{};
  Result<void, std::string> load_ret =
      loader.load(option.compilation_database_path, filter);
  if (load_ret.nok()) {
    spdlog::error(load_ret.take_error());
    std::exit(1);
  }
  spdlog::info("load compilation database successfully");

  Scheduler scheduler{8U};

  std::vector<Promise<loader::Loader::AstUnits>> create_ast_promises =
      loader.create_ast(scheduler);

  std::vector<Promise<void>> analysis_promises{};
  for (Promise<loader::Loader::AstUnits> &promise : create_ast_promises) {
    auto fn = [&promise, &filter, &storage]() -> void {
      loader::Loader::AstUnits &ast_units = promise->wait();
      for (std::unique_ptr<clang::ASTUnit> &ast : ast_units) {
        analysis::Context context{.m_ast_unit = ast.get(),
                                  .m_filter = &filter,
                                  .m_storage = &storage};
        analysis::InheritanceAnalysis{context}.start();
        ast.reset();
      }
    };
    analysis_promises.push_back(Promise<void>{
        new Task(AnalyzingPriority, fn, {&promise->get_task()}, scheduler)});
  }

  for (Promise<void> &promise : analysis_promises) {
    promise->wait();
  }
  spdlog::info("full analysis finished");
}
