add_library(ast_renderer STATIC src/gl_renderer.cpp)
target_include_directories(ast_renderer PUBLIC include)
target_link_libraries(ast_renderer PUBLIC ast_common SDL3::SDL3 glad::glad OpenGL::GL)
