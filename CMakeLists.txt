cmake_minimum_required(VERSION 3.28)

# --- Environment sanity check (must run before project()) ---
if(NOT DEFINED ENV{VCPKG_ROOT} OR "$ENV{VCPKG_ROOT}" STREQUAL "")
  message(FATAL_ERROR
    "VCPKG_ROOT environment variable is not set.\n"
    "  git clone https://github.com/microsoft/vcpkg.git $ENV{HOME}/vcpkg\n"
    "  $ENV{HOME}/vcpkg/bootstrap-vcpkg.sh\n"
    "  export VCPKG_ROOT=$ENV{HOME}/vcpkg\n"
    "Then re-run configure in a shell where VCPKG_ROOT is visible.")
endif()
if(NOT EXISTS "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
  message(FATAL_ERROR
    "VCPKG_ROOT is set to '$ENV{VCPKG_ROOT}' but the toolchain file is missing.\n"
    "Did bootstrap-vcpkg.sh finish successfully?")
endif()

project(asterism LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

# --- FetchContent pins -------------------------------------------------------

include(FetchContent)

FetchContent_Declare(entt
  GIT_REPOSITORY https://github.com/skypjack/entt.git
  GIT_TAG        85c6bba014049b5de8fad49d25424df2f1f6a8c1  # v4.0.0
)
FetchContent_Declare(imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        b916186e66aa3c02b72c6ca17455516512ed8049  # v1.93.0-docking
)
FetchContent_Declare(tomlplusplus
  GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
  GIT_TAG        30172438cee64926dc41fdd9c11fb3ba5b2ba9de  # v3.4.0
)
FetchContent_Declare(spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG        79524ddd08a4ec981b7fea76afd08ee05f83755d  # v1.17.0
)
FetchContent_Declare(Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        08092139210881f01a34e60c5f2fd6afa0ea4024  # v3.16.1
)
FetchContent_Declare(stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG        9600522de53973336fa033a289858a4354cbb04c  # master
)
FetchContent_Declare(miniaudio
  GIT_REPOSITORY https://github.com/mackron/miniaudio.git
  GIT_TAG        877ea4a7c2601c7b0f2a44e83dfac42e6470fea6  # 0.11.26
)

# --- Dependencies ------------------------------------------------------------
find_package(SDL3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

FetchContent_MakeAvailable(entt imgui tomlplusplus spdlog Catch2)

# ImGui is source-only; define a small static lib for it.
add_library(imgui STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui PUBLIC SDL3::SDL3 glad::glad OpenGL::GL)

# --- Modules (order matters: leaves first) -----------------------------------
add_subdirectory(src/common)
add_subdirectory(src/content)
add_subdirectory(src/sim)
add_subdirectory(src/session)
add_subdirectory(src/platform)
add_subdirectory(src/renderer)
add_subdirectory(src/ui)
add_subdirectory(src/engine)
add_subdirectory(app)

# --- Tests (M1+) -------------------------------------------------------------
option(AST_BUILD_TESTS "Build tests" ON)
if(AST_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
