add_library(ast_content STATIC
  src/toml_provider.cpp
  src/content_database.cpp
)
target_include_directories(ast_content PUBLIC include)
target_link_libraries(ast_content PUBLIC ast_common tomlplusplus::tomlplusplus)
