add_executable(inheritance_test
  inheritance_test.cpp
)
target_link_libraries(inheritance_test
  GTest::gtest_main
  GTest::gmock_main
  ${PROJECT_NAME}-lib
)

include(GoogleTest)
gtest_discover_tests(inheritance_test)
