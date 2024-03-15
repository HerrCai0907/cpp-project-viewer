aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/analysis cpp_project_check_lib_srcs)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/loader cpp_project_check_lib_srcs)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/persistence cpp_project_check_lib_srcs)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/server cpp_project_check_lib_srcs)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/utils cpp_project_check_lib_srcs)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} cpp_project_check_main_srcs)

add_library(${PROJECT_NAME}-lib
  ${cpp_project_check_lib_srcs}
)

llvm_map_components_to_libnames(llvm_libs
  support
)
set(clang_libs
  clangTooling
)

target_link_libraries(${PROJECT_NAME}-lib
  PUBLIC
  ${llvm_libs}
  ${clang_libs}
  spdlog::spdlog
  httplib::httplib
  nlohmann_json::nlohmann_json
)

separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
target_compile_definitions(${PROJECT_NAME}-lib
  PUBLIC
  ${LLVM_DEFINITIONS_LIST}
)

target_include_directories(${PROJECT_NAME}-lib
  PUBLIC
  ${LLVM_INCLUDE_DIRS}
  ${CLANG_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(${PROJECT_NAME}
  ${cpp_project_check_main_srcs}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  ${PROJECT_NAME}-lib
)
