cmake_minimum_required(VERSION 3.12)

add_subdirectory("StormLib")
add_subdirectory("squall")
add_subdirectory("SDL2")
add_subdirectory("tomlplusplus")
add_subdirectory("SDL_Image")
add_subdirectory("libsmackerdec")

set(squall_SOURCE_DIR ${squall_SOURCE_DIR} PARENT_SCOPE)
