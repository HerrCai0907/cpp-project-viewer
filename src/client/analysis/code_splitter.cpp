#include "cpjview/client/analysis/code_splitter.hpp"
#include "cpjview/client/analysis/naming.hpp"
#include "spdlog/spdlog.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "llvm/Support/raw_ostream.h"

namespace cpjview::analysis {

static std::string get_decl_raw_code(clang::Decl *decl) {
  std::string ss{};
  llvm::raw_string_ostream os{ss};
  decl->print(os);
  return ss;
}

namespace {

class CodeSplitterVisitor
    : public clang::RecursiveASTVisitor<CodeSplitterVisitor> {
  using super = clang::RecursiveASTVisitor<CodeSplitterVisitor>;
  Context const &m_context;

public:
  explicit CodeSplitterVisitor(Context const &context)
      : clang::RecursiveASTVisitor<CodeSplitterVisitor>{}, m_context(context) {}

  bool VisitCXXRecordDecl(clang::CXXRecordDecl *decl) {
    handleRecord(decl);
    return true;
  }

private:
  void handleRecord(clang::CXXRecordDecl *decl) {
    std::string name = get_record_name(decl);
    std::string code = get_decl_raw_code(decl);
    spdlog::trace("[code splitter] find c++ record {}, code:\n{}", name,
                  decl->getSourceRange().printToString(
                      m_context.m_ast_unit->getSourceManager()));
    m_context.m_storage->add_code(name, code);
  }
};

} // namespace

void CodeSplitter::start_impl() {
  CodeSplitterVisitor{get_context()}.TraverseAST(
      get_context().m_ast_unit->getASTContext());
}

} // namespace cpjview::analysis
