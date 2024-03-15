#pragma once

#include "cpjview/loader/filter.hpp"
#include "cpjview/persistence/persistence.hpp"
#include "clang/Frontend/ASTUnit.h"
#include "llvm/ADT/StringRef.h"

namespace cpjview::analysis {

struct Context {
  clang::ASTUnit *m_ast_unit;
  loader::Filter const *m_filter;
  persistence::Storage *m_storage;

  bool need_analysis(clang::Decl *decl) const;
  llvm::StringRef get_file_name() const {
    return m_ast_unit->getMainFileName();
  }
};

class AnalysisAction {
public:
  explicit AnalysisAction(Context context) : m_context(context) {}

  void start();
  virtual const char *get_analysis_name() = 0;

protected:
  Context const &get_context() { return m_context; }
  virtual void start_impl() = 0;

private:
  Context m_context;
};

} // namespace cpjview::analysis
