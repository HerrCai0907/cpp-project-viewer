#include "cpjview/client/analysis/code_splitter.hpp"
#include "cpjview/client/analysis/context_controlled_visitor.hpp"
#include "cpjview/client/analysis/naming.hpp"
#include "cpjview/protocol/label.hpp"
#include "spdlog/spdlog.h"

namespace cpjview::analysis {

static std::string get_decl_raw_code(clang::Decl *decl) {
  std::string ss{};
  llvm::raw_string_ostream os{ss};
  decl->print(os);
  return ss;
}

namespace {

class CodeSplitterVisitor
    : public ContextControlledVisitor<CodeSplitterVisitor> {
  using super = ContextControlledVisitor<CodeSplitterVisitor>;

public:
  explicit CodeSplitterVisitor(Context const &context)
      : ContextControlledVisitor<CodeSplitterVisitor>{context} {}

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
                      get_context().m_ast_unit->getSourceManager()));
    get_context().m_storage->add_source_code_relationship(name, code);
    get_context().m_storage->mark_label(name, protocol::LabelKind::Record);
  }
};

} // namespace

void CodeSplitter::start_impl() {
  CodeSplitterVisitor{get_context()}.TraverseAST(
      get_context().m_ast_unit->getASTContext());
}

} // namespace cpjview::analysis
