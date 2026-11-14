add_library(ast_ui STATIC src/ui_context.cpp)
target_include_directories(ast_ui PUBLIC include)
target_link_libraries(ast_ui PUBLIC ast_common ast_sim ast_session imgui)
