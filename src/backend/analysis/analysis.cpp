#include "cpjview/analysis/analysis.hpp"
#include "spdlog/spdlog.h"

namespace cpjview::analysis {

bool Context::need_analysis(clang::Decl *decl) const {
  return m_filter->validate(
      m_ast_unit->getSourceManager().getFilename(decl->getBeginLoc()));
}

void AnalysisAction::start() {
  spdlog::trace("analysis {} for '{}' start", get_analysis_name(),
                m_context.get_file_name());
  start_impl();
  spdlog::debug("analysis {} for '{}' finish", get_analysis_name(),
                m_context.get_file_name());
}

} // namespace cpjview::analysis
