add_library(ast_session STATIC
  src/session_stub.cpp
)
target_include_directories(ast_session PUBLIC include)
target_link_libraries(ast_session PUBLIC ast_common ast_sim ast_content)
target_compile_features(ast_session PUBLIC cxx_std_23)
