# =============================================================================
# CMakeLists.txt - Build Configuration for GameEngine
# =============================================================================
#
# This is the CMake build system configuration file for the GameEngine project.
# It defines how the project is compiled, what libraries it depends on, and
# how the final executable is assembled.
#
# PROJECT OVERVIEW:
#   GameEngine is a 2D game engine built on top of SDL2 (Simple DirectMedia
#   Layer). This file orchestrates the entire build process, from setting the
#   required CMake version and C++ standard to locating the SDL2 library,
#   defining the executable target, linking dependencies, and ensuring runtime
#   DLLs are available next to the built binary.
#
# BUILD REQUIREMENTS:
#   - CMake 3.20 or higher
#   - A C++17-compliant compiler (MSVC, GCC, or Clang)
#   - SDL2 development libraries installed at C:/SDL2
#     (headers under C:/SDL2/include, 64-bit libs under C:/SDL2/lib/x64)
#
# HOW TO BUILD:
#   1. Create a build directory:        mkdir build && cd build
#   2. Configure with CMake:            cmake ..
#   3. Compile:                         cmake --build .
#   The resulting GameEngine.exe will be placed in the build output directory
#   alongside a copied SDL2.dll so it can run without manual DLL placement.
#
# SDL2 INTEGRATION:
#   SDL2 is located manually via SDL2_DIR rather than through find_package,
#   giving full control over which SDL2 installation is used. The include
#   and library directories are added globally so all future source files
#   added to the target will automatically resolve SDL2 headers and symbols.
#
# POST-BUILD STEP:
#   A custom post-build command copies SDL2.dll from the SDL2 lib directory
#   into the same folder as the compiled executable. This ensures the program
#   can be launched directly from the build directory without needing SDL2.dll
#   to be on the system PATH.
# =============================================================================

cmake_minimum_required(VERSION 3.20)
project(GameEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SDL2_DIR "C:/SDL2")
include_directories(${SDL2_DIR}/include)
link_directories(${SDL2_DIR}/lib/x64)

add_executable(GameEngine
    src/main.cpp
)

target_link_libraries(GameEngine SDL2main SDL2)

add_custom_command(TARGET GameEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${SDL2_DIR}/lib/x64/SDL2.dll"
    $<TARGET_FILE_DIR:GameEngine>)
