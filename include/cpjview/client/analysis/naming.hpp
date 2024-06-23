#pragma once

#include "clang/AST/DeclCXX.h"
#include <string>

namespace cpjview::analysis {

std::string get_record_name(const clang::CXXRecordDecl *decl);

}
