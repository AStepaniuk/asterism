add_library(ast_sim STATIC
  src/sim_stub.cpp
)
target_include_directories(ast_sim PUBLIC include)
target_link_libraries(ast_sim PUBLIC ast_common ast_content EnTT::EnTT)
target_compile_features(ast_sim PUBLIC cxx_std_23)