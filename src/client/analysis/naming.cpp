

#include "cpjview/client/analysis/naming.hpp"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclTemplate.h"
#include "llvm/ADT/StringExtras.h"
#include <format>
#include <string>

namespace cpjview::analysis {

static std::string
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

static std::string get_specialization_name(
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

} // namespace cpjview::analysis
