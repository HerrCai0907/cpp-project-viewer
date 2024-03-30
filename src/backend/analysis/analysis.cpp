#include "cpjview/analysis/analysis.hpp"
#include "spdlog/spdlog.h"

namespace cpjview::analysis {

bool Context::need_analysis(clang::Decl *decl) const {
  if (decl->getLocation().isInvalid()) {
    return false;
  }
  clang::SourceManager const &source_manager = m_ast_unit->getSourceManager();
  clang::SourceLocation spelling_location =
      source_manager.getSpellingLoc(decl->getLocation());
  if (source_manager.isWrittenInScratchSpace(spelling_location)) {
    return false;
  }
  llvm::StringRef file_path = source_manager.getFilename(spelling_location);
  if (file_path.empty()) {
    spdlog::error("invalid spelling loc");
    source_manager.getSpellingLoc(decl->getLocation()).dump(source_manager);
    return false;
  }
  return m_filter->validate(file_path);
}

void AnalysisAction::start() {
  spdlog::trace("[analysis] {} for '{}' start", get_analysis_name(),
                m_context.get_file_name());
  start_impl();
  spdlog::debug("analysis {} for '{}' finish", get_analysis_name(),
                m_context.get_file_name());
}

} // namespace cpjview::analysis
