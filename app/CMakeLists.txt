add_executable(ast_app main.cpp)
target_link_libraries(ast_app PRIVATE ast_engine)
target_compile_features(ast_app PRIVATE cxx_std_23)
