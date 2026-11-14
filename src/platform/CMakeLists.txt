add_library(ast_platform STATIC src/sdl3_platform.cpp)
target_include_directories(ast_platform PUBLIC include)
target_link_libraries(ast_platform PUBLIC ast_common SDL3::SDL3)
