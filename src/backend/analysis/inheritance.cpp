
#include "cpjview/analysis/inheritance.hpp"
#include "spdlog/spdlog.h"
#include "clang/AST/Decl.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Casting.h"

namespace cpjview::analysis {

namespace {

std::string get_record_name(const clang::ClassTemplateDecl *template_decl) {
  // in template
  std::string name = template_decl->getQualifiedNameAsString();
  name += "<";
  llvm::SmallVector<std::string> param_names{};
  for (clang::NamedDecl *template_param :
       *template_decl->getTemplateParameters()) {
    param_names.push_back(template_param->getQualifiedNameAsString());
  }
  name += llvm::join(param_names, ",");
  name += ">";
  return name;
}

std::string get_record_name(const clang::CXXRecordDecl *decl) {
  // in template
  clang::ClassTemplateDecl *template_decl = decl->getDescribedClassTemplate();
  if (template_decl) {
    return get_record_name(template_decl);
  }
  return decl->getQualifiedNameAsString();
}

std::string get_type_name(clang::QualType const &type) {
  const clang::CXXRecordDecl *record_decl =
      type->getUnqualifiedDesugaredType()->getAsCXXRecordDecl();
  if (const auto *specialization_decl =
          llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(record_decl)) {
    return get_record_name(specialization_decl->getSpecializedTemplate());
  }
  return get_record_name(record_decl);
}

class InheritanceVisitor
    : public clang::RecursiveASTVisitor<InheritanceVisitor> {
  using super = clang::RecursiveASTVisitor<InheritanceVisitor>;

public:
  explicit InheritanceVisitor(Context const &context)
      : clang::RecursiveASTVisitor<InheritanceVisitor>{}, m_context(context) {}

  bool TraverseDecl(clang::Decl *decl) {
    if (m_context.need_analysis(decl)) {
      super::TraverseDecl(decl);
    }
    return true;
  }

  bool VisitCXXRecordDecl(clang::CXXRecordDecl *decl) {
    if (llvm::isa<clang::ClassTemplateSpecializationDecl>(decl)) {
      return true;
    }
    if (!decl->hasDefinition()) {
      // ignore declaration
      return true;
    }

    std::string name = get_record_name(decl);
    spdlog::trace("find c++ record {}", name);
    for (auto base : decl->bases()) {
      std::string base_name = get_type_name(base.getType());
      m_context.m_storage->add_inheritance_relationship(name, base_name);
      spdlog::debug("insert inheritance relationship: {} <- {}", name,
                    base_name);
    }
    return true;
  }

private:
  Context const &m_context;
};

} // namespace

void InheritanceAnalysis::start_impl() {
  InheritanceVisitor{get_context()}.TraverseAST(
      get_context().m_ast_unit->getASTContext());
}

} // namespace cpjview::analysis
