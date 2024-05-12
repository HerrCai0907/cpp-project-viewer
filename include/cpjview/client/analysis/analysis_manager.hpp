#pragma once

#include "cpjview/client/loader/filter.hpp"
#include "cpjview/client/loader/loader.hpp"
#include "cpjview/client/sync/sync.hpp"
#include "cpjview/common/scheduler.hpp"

namespace cpjview::analysis {

class AnalysisManager {
public:
  explicit AnalysisManager(Scheduler &scheduler, loader::Filter &filter,
                           sync::ISync &storage)
      : m_scheduler(scheduler), m_filter(filter), m_storage(storage) {}

  void analysis(std::vector<Promise<void>> &analysis_promises,
                Promise<loader::Loader::AstUnits> &&promise);

private:
  Scheduler &m_scheduler;
  loader::Filter &m_filter;
  sync::ISync &m_storage;
};

} // namespace cpjview::analysis
