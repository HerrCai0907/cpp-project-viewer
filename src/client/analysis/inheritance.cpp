
#include "cpjview/client/analysis/inheritance.hpp"
#include "cpjview/client/analysis/naming.hpp"
#include "spdlog/spdlog.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "llvm/Support/Casting.h"

namespace cpjview::analysis {

namespace {

class InheritanceVisitor
    : public clang::RecursiveASTVisitor<InheritanceVisitor> {
  using super = clang::RecursiveASTVisitor<InheritanceVisitor>;
  Context const &m_context;

public:
  explicit InheritanceVisitor(Context const &context)
      : clang::RecursiveASTVisitor<InheritanceVisitor>{}, m_context(context) {}

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

  bool VisitCXXRecordDecl(clang::CXXRecordDecl *decl) {
    if (clang::ClassTemplateDecl *template_decl =
            decl->getDescribedClassTemplate()) {
      for (clang::ClassTemplateSpecializationDecl *specialization_decl :
           template_decl->specializations()) {
        handleRecord(specialization_decl);
      }
    } else {
      handleRecord(decl);
    }
    return true;
  }

private:
  void handleRecord(clang::CXXRecordDecl *decl) {
    if (!decl->hasDefinition() || decl->isDependentType() ||
        llvm::isa<clang::ClassTemplatePartialSpecializationDecl>(decl)) {
      return;
    }
    std::string name = get_record_name(decl);
    spdlog::trace("[inheritance] find c++ record {}", name);
    for (clang::CXXBaseSpecifier &base : decl->bases()) {
      std::string base_name =
          get_record_name(base.getType()->getAsCXXRecordDecl());
      m_context.m_storage->add_inheritance_relationship(name, base_name);
      spdlog::debug("insert inheritance relationship: {} <- {}", name,
                    base_name);
    }
  }
};

} // namespace

void InheritanceAnalysis::start_impl() {
  InheritanceVisitor{get_context()}.TraverseAST(
      get_context().m_ast_unit->getASTContext());
}

} // namespace cpjview::analysis
