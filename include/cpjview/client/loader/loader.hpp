#pragma once

#include "cpjview/client/loader/filter.hpp"
#include "cpjview/common/result.hpp"
#include "cpjview/common/scheduler.hpp"
#include "clang/Frontend/ASTUnit.h"
#include "clang/Tooling/JSONCompilationDatabase.h"
#include "llvm/ADT/StringRef.h"

namespace cpjview::loader {

class Loader {
public:
  Result<void, std::string> load(llvm::StringRef compilation_database_path,
                                 Filter const &filter);

  using AstUnits = std::vector<std::unique_ptr<clang::ASTUnit>>;
  std::vector<Promise<AstUnits>> create_ast(Scheduler &scheduler);

private:
  using JSONCompilationDatabase = clang::tooling::JSONCompilationDatabase;

  std::vector<std::string> m_target_files{};
  std::unique_ptr<JSONCompilationDatabase> m_data_base{};
};

} // namespace cpjview::loader
