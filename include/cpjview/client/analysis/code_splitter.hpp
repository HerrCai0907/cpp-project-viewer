#pragma once

#include "cpjview/client/analysis/analysis.hpp"

namespace cpjview::analysis {

class CodeSplitter : public AnalysisAction {
public:
  using AnalysisAction::AnalysisAction;

  const char *get_analysis_name() override { return "code splitter"; }

  void start_impl() override;
};

} // namespace cpjview::analysis
