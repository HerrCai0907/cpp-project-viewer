
#include "cpjview/client/analysis/inheritance.hpp"
#include "spdlog/spdlog.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/TemplateName.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Casting.h"
#include <__format/format_functions.h>

namespace cpjview::analysis {

namespace {

std::string
get_template_name(llvm::ArrayRef<clang::TemplateArgument> const &args,
                  clang::PrintingPolicy const &policy) {
  llvm::SmallVector<std::string> param_names{};
  for (clang::TemplateArgument const &template_param : args) {
    if (template_param.getKind() == clang::TemplateArgument::ArgKind::Pack) {
      param_names.push_back(std::format(
          "{{{}}}",
          get_template_name(template_param.getPackAsArray(), policy)));
    } else {
      std::string param_name{};
      llvm::raw_string_ostream ss{param_name};
      template_param.print(policy, ss, true);
      param_names.push_back(param_name);
    }
  }
  return llvm::join(param_names, ", ");
}

std::string get_specialization_name(
    const clang::ClassTemplateSpecializationDecl *template_decl) {
  return std::format(
      "{}<{}>", template_decl->getQualifiedNameAsString(),
      get_template_name(
          template_decl->getTemplateInstantiationArgs().asArray(),
          clang::PrintingPolicy{template_decl->getASTContext().getLangOpts()}));
}

std::string get_record_name(const clang::CXXRecordDecl *decl) {
  // in template
  if (const clang::ClassTemplateSpecializationDecl *template_decl =
          llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(decl)) {
    return get_specialization_name(template_decl);
  }
  return decl->getQualifiedNameAsString();
}

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
