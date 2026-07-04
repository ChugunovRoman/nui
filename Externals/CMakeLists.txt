cmake_minimum_required(VERSION 3.16)
project(nui-externals LANGUAGES C CXX)

set(SDL2_DISABLE_INSTALL ON CACHE BOOL "" FORCE)
set(SDL2_DISABLE_UNINSTALL ON CACHE BOOL "" FORCE)
set(SDL2TTF_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(SDL2 EXCLUDE_FROM_ALL)
add_subdirectory(SDL_ttf EXCLUDE_FROM_ALL)
add_subdirectory(pugixml EXCLUDE_FROM_ALL)
