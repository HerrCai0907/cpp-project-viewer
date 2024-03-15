#pragma once

#include "cpjview/analysis/analysis.hpp"

namespace cpjview::analysis {

class InheritanceAnalysis : public AnalysisAction {
public:
  using AnalysisAction::AnalysisAction;

  const char *get_analysis_name() override {
    return "inheritance relationship";
  }

  void start_impl() override;
};

} // namespace cpjview::analysis
