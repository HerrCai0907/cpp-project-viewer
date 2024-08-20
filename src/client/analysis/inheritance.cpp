
#include "cpjview/client/analysis/inheritance.hpp"
#include "cpjview/client/analysis/context_controlled_visitor.hpp"
#include "cpjview/client/analysis/naming.hpp"
#include "spdlog/spdlog.h"
#include "clang/AST/DeclTemplate.h"
#include "llvm/Support/Casting.h"

namespace cpjview::analysis {

namespace {

class InheritanceVisitor : public ContextControlledVisitor<InheritanceVisitor> {

public:
  explicit InheritanceVisitor(Context const &context)
      : ContextControlledVisitor{context} {}

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
      get_context().m_storage->add_inheritance_relationship(name, base_name);
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
