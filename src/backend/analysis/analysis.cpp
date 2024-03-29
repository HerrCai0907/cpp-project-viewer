#include "cpjview/analysis/analysis.hpp"
#include "spdlog/spdlog.h"

namespace cpjview::analysis {

bool Context::need_analysis(clang::Decl *decl) const {
  if (decl->getLocation().isInvalid()) {
    return false;
  }
  clang::SourceManager const &source_manager = m_ast_unit->getSourceManager();
  llvm::StringRef file_path = source_manager.getFilename(
      source_manager.getSpellingLoc(decl->getLocation()));
  return m_filter->validate(file_path);
}

void AnalysisAction::start() {
  spdlog::trace("analysis {} for '{}' start", get_analysis_name(),
                m_context.get_file_name());
  start_impl();
  spdlog::debug("analysis {} for '{}' finish", get_analysis_name(),
                m_context.get_file_name());
}

} // namespace cpjview::analysis
