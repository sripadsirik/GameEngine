cmake_minimum_required(VERSION 3.20)
project(GameEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SDL2_DIR "C:/SDL2")
set(SDL2_IMAGE_DIR "C:/SDL2_image")
set(SDL2_MIXER_DIR "C:/SDL2_mixer")

include_directories(${SDL2_DIR}/include)
include_directories(${SDL2_IMAGE_DIR}/include)
include_directories(${SDL2_MIXER_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include)

link_directories(${SDL2_DIR}/lib/x64)
link_directories(${SDL2_IMAGE_DIR}/lib/x64)
link_directories(${SDL2_MIXER_DIR}/lib/x64)

add_executable(GameEngine 
    src/main.cpp
)

target_link_libraries(GameEngine 
    SDL2main 
    SDL2
    SDL2_image
    SDL2_mixer
)

add_custom_command(TARGET GameEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${SDL2_DIR}/lib/x64/SDL2.dll"
    $<TARGET_FILE_DIR:GameEngine>)

add_custom_command(TARGET GameEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${SDL2_IMAGE_DIR}/lib/x64/SDL2_image.dll"
    $<TARGET_FILE_DIR:GameEngine>)

add_custom_command(TARGET GameEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${SDL2_MIXER_DIR}/lib/x64/SDL2_mixer.dll"
    $<TARGET_FILE_DIR:GameEngine>)

add_custom_command(TARGET GameEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets"
    $<TARGET_FILE_DIR:GameEngine>/assets)