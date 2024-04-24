aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} cpjview_common_srcs)

add_library(cpjview_common
  ${cpjview_common_srcs}
)

target_link_libraries(cpjview_common
  PUBLIC
  spdlog::spdlog
  httplib::httplib
  nlohmann_json::nlohmann_json
)

target_include_directories(cpjview_common
  PUBLIC
  ${LLVM_INCLUDE_DIRS}
  ${CLANG_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/include
)
