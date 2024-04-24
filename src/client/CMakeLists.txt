aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/analysis cpjview_client_srcs)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/loader cpjview_client_srcs)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/sync cpjview_client_srcs)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/utils cpjview_client_srcs)

add_library(cpjview_client_lib
  ${cpjview_client_srcs}
)

llvm_map_components_to_libnames(llvm_libs
  support
)
set(clang_libs
  clangTooling
)

target_link_libraries(cpjview_client_lib
  PUBLIC
  ${llvm_libs}
  ${clang_libs}
  cpjview_common
)

separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
target_compile_definitions(cpjview_client_lib
  PUBLIC
  ${LLVM_DEFINITIONS_LIST}
)

target_include_directories(cpjview_client_lib
  PUBLIC
  ${LLVM_INCLUDE_DIRS}
  ${CLANG_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} cpjview_client_main_srcs)

add_executable(cpjview_client
  ${cpjview_client_main_srcs}
)
target_link_libraries(cpjview_client
  PRIVATE
  cpjview_client_lib
)
