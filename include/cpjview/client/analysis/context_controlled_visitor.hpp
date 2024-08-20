#pragma once

#include "cpjview/client/analysis/analysis.hpp"
#include "clang/AST/Decl.h"
#include "clang/AST/RecursiveASTVisitor.h"

namespace cpjview::analysis {

template <class Derived>
class ContextControlledVisitor : public clang::RecursiveASTVisitor<Derived> {
  using super = clang::RecursiveASTVisitor<Derived>;
  Context const &m_context;

public:
  explicit ContextControlledVisitor(Context const &context)
      : clang::RecursiveASTVisitor<Derived>{}, m_context(context) {}

  bool TraverseNamespaceDecl(clang::NamespaceDecl *decl) {
    if (m_context.need_analysis(decl)) {
      super::TraverseNamespaceDecl(decl);
    }
    return true;
  }
  bool TraverseCXXRecordDecl(clang::CXXRecordDecl *decl) {
    if (m_context.need_analysis(decl)) {
      super::TraverseCXXRecordDecl(decl);
    }
    return true;
  }

  Context const &get_context() { return m_context; }
};

} // namespace cpjview::analysis
