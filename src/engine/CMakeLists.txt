add_library(ast_engine STATIC src/application.cpp src/game_loop.cpp)
target_include_directories(ast_engine PUBLIC include)
target_link_libraries(ast_engine PUBLIC ast_common ast_session ast_platform ast_renderer ast_ui)
