cmake_minimum_required(VERSION 3.24)
# Project name and version
project(endless-project VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Source directories
set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

# Source files
file(GLOB SRC_FILES ${SRC_DIR}/*.cpp)

# Main executable
add_executable(${PROJECT_NAME} ${SRC_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR})

# FetchContent setup
include(FetchContent)

# ==== IMGUI ====
find_package(imgui QUIET)
if (NOT imgui_FOUND)
    message(STATUS "Fetching Dear ImGui from GitHub")
    FetchContent_Declare(
        ImGui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.91.3
    )
    FetchContent_MakeAvailable(ImGui)
    add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
    )
    target_include_directories(imgui INTERFACE ${imgui_SOURCE_DIR})
else()
    message(STATUS "Using local Dear ImGui library")
endif()

# ==== RAYLIB ====
find_package(raylib 5.0 QUIET)
if (NOT raylib_FOUND)
    message(STATUS "Fetching raylib from GitHub")
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.0
    )
    FetchContent_MakeAvailable(raylib)
else()
    message(STATUS "Using local raylib library")
endif()

# ==== RLIMGUI ====
find_package(rlimgui QUIET)
if (NOT rlimgui_FOUND)
    message(STATUS "Fetching rlImGui from GitHub")
    FetchContent_Declare(
        rlImGui
        GIT_REPOSITORY https://github.com/raylib-extras/rlImGui
        GIT_TAG c7303258e045b4698c3e0b930b71956b5bbe7dff
    )
    FetchContent_MakeAvailable(rlImGui)
    add_library(rlimgui STATIC ${rlimgui_SOURCE_DIR}/rlImGui.cpp)
    target_include_directories(rlimgui INTERFACE ${rlimgui_SOURCE_DIR})
    target_link_libraries(rlimgui PRIVATE imgui raylib)
else()
    message(STATUS "Using local rlImGui library")
endif()

# Link libraries to the project
target_link_libraries(${PROJECT_NAME} PRIVATE imgui rlimgui raylib)

# ==== WASM Configuration ====
if (CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_GLFW=3 -s ASYNCIFY -s WASM=1")
    target_link_options(${PROJECT_NAME} PRIVATE
        "-s USE_GLFW=3"
        "-s ASYNCIFY"
        "-s WASM=1"
    )
    message(STATUS "Building for WebAssembly")
endif()
