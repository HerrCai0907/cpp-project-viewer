#include "cpjview/client/analysis/inheritance.hpp"
#include "cpjview/client/loader/filter.hpp"
#include "cpjview/client/sync/sync.hpp"
#include "clang/Tooling/Tooling.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <string_view>

using namespace cpjview;

struct FilterMock : public loader::Filter {
  FilterMock() : loader::Filter({}) {}

  bool validate_impl(std::string const &) const override { return true; }
};

struct Inheritance {
  std::string derived;
  std::string base;
  bool operator==(Inheritance const &o) const {
    return derived == o.derived && base == o.base;
  }
  bool operator!=(Inheritance const &o) const { return !((*this) == o); }
};

struct TestSync : public sync::ISync {
  MOCK_METHOD(void, add_inheritance_relationship,
              (std::string const &derived, std::string const &base),
              (override));
  MOCK_METHOD(void, add_code,
              (std::string const &name, std::string const &code), (override));
};

struct TestParm {
  const char *code;
  std::vector<Inheritance> expects;
};

struct InheritanceAnalysisTest : public ::testing::TestWithParam<TestParm> {
  std::vector<Inheritance> do_analysis(std::string_view code) {
    std::unique_ptr<clang::ASTUnit> ast =
        clang::tooling::buildASTFromCode(code);
    FilterMock filter{};

    analysis::Context context{
        .m_ast_unit = ast.get(),
        .m_filter = &filter,
        .m_storage = &m_storage,
    };
    analysis::InheritanceAnalysis analysis{context};

    std::vector<Inheritance> ret{};
    ON_CALL(m_storage, add_inheritance_relationship)
        .WillByDefault(
            [&](std::string const &derived, std::string const &base) -> void {
              ret.push_back(Inheritance{.derived = derived, .base = base});
            });

    analysis.start_impl();
    std::stringstream ss{};
    ss << "\ncode:\n" << code << "\nresult:\n";
    for (Inheritance const &pair : ret) {
      ss << pair.derived << " <- " << pair.base << "\n";
    }
    m_trace =
        std::make_unique<::testing::ScopedTrace>(__FILE__, __LINE__, ss.str());
    return ret;
  }
  TestSync m_storage{};
  std::unique_ptr<::testing::ScopedTrace> m_trace{};
};

TEST_P(InheritanceAnalysisTest, 1) {
  auto relationships = do_analysis(GetParam().code);

  EXPECT_THAT(relationships,
              testing::UnorderedElementsAreArray(GetParam().expects));
}

INSTANTIATE_TEST_SUITE_P(
    InheritanceAnalysisTest, InheritanceAnalysisTest,
    ::testing::ValuesIn<std::vector<TestParm>>(
        {
            {
                .code = R"(  
class A {};
class B : A {};
)",
                .expects = {{.derived = "B", .base = "A"}},
            },
            {
                .code = R"(
namespace ns {
  class A {};
  class B : A {};
}
)",
                .expects = {{.derived = "ns::B", .base = "ns::A"}},
            },
            {
                .code = R"(
class A {};
class Out {
  class B : A {};
};
)",
                .expects = {{.derived = "Out::B", .base = "A"}},
            },
            {
                .code = "class A;",
                .expects = {},
            },
            {
                .code = R"(
class A {};
using Align = A;
class B : Align {};
)",
                .expects = {{.derived = "B", .base = "A"}},
            },
            {
                .code = R"(
class A {};
template<class T> class B : A {};
B<int> b1;
B<char> b2;
)",
                .expects =
                    {
                        {.derived = "B<int>", .base = "A"},
                        {.derived = "B<char>", .base = "A"},
                    },
            },
            {
                .code = R"(
template<class> class A {};
class B : A<int> {};
)",
                .expects = {{.derived = "B", .base = "A<int>"}},
            },
            {
                .code = R"(
template<class> class A {};
using i32 = int;
class B : A<i32> {};
)",
                .expects = {{.derived = "B", .base = "A<int>"}},
            },
            {
                .code = R"(
template<int> class A {};
class B : A<12345> {};
)",
                .expects = {{.derived = "B", .base = "A<12345>"}},
            },
            {
                .code = R"(
template<class, class...> class A {};
class B : A<int,long,unsigned> {};
)",
                .expects = {{.derived = "B",
                             .base = "A<int, {long, unsigned int}>"}},
            },
            {
                .code = R"(
template<class> class A {};
class B : A<void> {};
class C : A<int> {};
)",
                .expects =
                    {
                        {.derived = "B", .base = "A<void>"},
                        {.derived = "C", .base = "A<int>"},
                    },
            },
            {
                .code = R"(
template<class> struct A {
    template<class> struct B {};
};
class C : A<int>::B<int> {};
)",
                .expects =
                    {
                        {.derived = "C", .base = "A<int>::B<int>"},
                    },
            },

        }));
