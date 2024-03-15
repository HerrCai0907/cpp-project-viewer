#include "cpjview/loader/loader.hpp"
#include "cpjview/loader/filter.hpp"
#include "spdlog/spdlog.h"
#include "clang/Tooling/CompilationDatabase.h"
#include "clang/Tooling/JSONCompilationDatabase.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/ADT/StringRef.h"

namespace cpjview::loader {

Result<void, std::string>
Loader::load(llvm::StringRef compilation_database_path, Filter const &filter) {
  std::string error_message{};
  m_data_base = JSONCompilationDatabase::loadFromFile(
      compilation_database_path, error_message,
      clang::tooling::JSONCommandLineSyntax::AutoDetect);
  if (m_data_base == nullptr) {
    return Result<void, std::string>::failed(error_message);
  }
  spdlog::debug("load compilation database {} successfully",
                compilation_database_path);
  // file_path is absolute path
  for (std::string const &file_path : m_data_base->getAllFiles()) {
    std::string normalize_file_path{};
    if (filter.validate(file_path)) {
      spdlog::debug("add target file: {}", file_path);
      m_target_files.push_back(file_path);
    }
  }
  return Result<void, std::string>::success();
}

std::vector<std::unique_ptr<clang::ASTUnit>> Loader::create_ast() {
  std::vector<std::unique_ptr<clang::ASTUnit>> asts{};
  for (std::string const &file : m_target_files) {
    spdlog::trace("generate ast for '{}'", file);
    clang::tooling::ClangTool tool{*m_data_base, {file}};
    switch (tool.buildASTs(asts)) {
    case 0:
      spdlog::debug("generate ast for '{} successfully'", file);
      break;
    case 1:
      spdlog::error("generate ast for '{} failed'", file);
      break;
    case 2:
      spdlog::warn("generate ast for '{} skipped'", file);
      break;
    }
  }
  return asts;
}

} // namespace cpjview::loader
