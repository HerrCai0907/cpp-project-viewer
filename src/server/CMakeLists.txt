aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/http cpjview_server_srcs)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/persistence cpjview_server_srcs)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} cpjview_server_srcs)

add_executable(cpjview_server
  ${cpjview_server_srcs}
)

target_link_libraries(cpjview_server
  PUBLIC
  cpjview_common
  sqlite3
)

target_include_directories(cpjview_server
  PUBLIC
  ${LLVM_INCLUDE_DIRS}
  ${CLANG_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
)
