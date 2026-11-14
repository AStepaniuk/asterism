add_library(ast_common STATIC
  src/math/fixed.cpp
  src/math/hex.cpp
  src/rng/pcg32.cpp
)
target_include_directories(ast_common PUBLIC include)
target_link_libraries(ast_common PUBLIC spdlog::spdlog)
target_compile_features(ast_common PUBLIC cxx_std_23)
