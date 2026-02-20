cmake_minimum_required(VERSION 3.20)
project(GameEngine)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL2
set(SDL2_DIR "C:/SDL2")
include_directories(${SDL2_DIR}/include)
link_directories(${SDL2_DIR}/lib/x64)

# Add our source files
add_executable(GameEngine 
    src/main.cpp
)

# Link SDL2
target_link_libraries(GameEngine SDL2main SDL2)

# Copy SDL2.dll to build directory (so the .exe can find it)
add_custom_command(TARGET GameEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${SDL2_DIR}/lib/x64/SDL2.dll"
    $<TARGET_FILE_DIR:GameEngine>)