#include "cpjview/client/analysis/analysis_manager.hpp"
#include "cpjview/client/analysis/analysis.hpp"
#include "cpjview/client/analysis/code_splitter.hpp"
#include "cpjview/client/analysis/inheritance.hpp"
#include "cpjview/client/loader/loader.hpp"
#include "cpjview/client/utils/task_priority.hpp"
#include "cpjview/common/scheduler.hpp"

namespace cpjview::analysis {

void AnalysisManager::analysis(std::vector<Promise<void>> &analysis_promises,
                               Promise<loader::Loader::AstUnits> &&promise) {
  auto fn = [promise, this]() -> void {
    loader::Loader::AstUnits &ast_units = promise.get_task()->wait_for_value();
    for (std::unique_ptr<clang::ASTUnit> &ast : ast_units) {
      analysis::Context context{.m_ast_unit = ast.get(),
                                .m_filter = &m_filter,
                                .m_storage = &m_storage};
      analysis::InheritanceAnalysis{context}.start();
      analysis::CodeSplitter{context}.start();
    }
  };
  analysis_promises.push_back(Promise<void>{
      AnalyzingPriority, fn, {promise.get_task().get()}, m_scheduler});
}

} // namespace cpjview::analysis
