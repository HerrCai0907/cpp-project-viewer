add_subdirectory(httplib)

set(JSON_ImplicitConversions off)
add_subdirectory(json)

add_subdirectory(spdlog)

add_library(sqlite3 SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/sqlite3/sqlite3.c
)
target_include_directories(sqlite3 SYSTEM PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/sqlite3
)
