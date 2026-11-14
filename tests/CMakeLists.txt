add_executable(ast_tests unit/smoke_test.cpp)
target_link_libraries(ast_tests PRIVATE Catch2::Catch2WithMain ast_common)
include(Catch)
catch_discover_tests(ast_tests)
