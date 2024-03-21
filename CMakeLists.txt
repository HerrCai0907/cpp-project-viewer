cmake_minimum_required(VERSION 3.20.0)
project(cpp-project-view)

option(CPJVIEW_ENABLE_TEST "enable test" off)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(LLVM REQUIRED PATHS ${PROJECT_SOURCE_DIR}/third_party/llvm-release NO_DEFAULT_PATH)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

find_package(CLANG REQUIRED PATHS ${PROJECT_SOURCE_DIR}/third_party/llvm-release NO_DEFAULT_PATH)

message(STATUS "Found LLVM ${CLANG_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${CLANG_DIR}")

add_subdirectory(third_party)
add_subdirectory(src)

if(CPJVIEW_ENABLE_TEST)
  add_subdirectory(tests)
endif()