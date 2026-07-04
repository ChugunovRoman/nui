cmake_minimum_required(VERSION 3.16)
project(nui-example LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ── Locate the library sources ───────────────────────────────────
set(NUI_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../src)

# ── Dependencies (same as library) ──────────────────────────────
include(FetchContent)

FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-2.30.12
    GIT_SHALLOW    TRUE
)
set(SDL2_DISABLE_INSTALL ON CACHE BOOL "" FORCE)
set(SDL2_DISABLE_UNINSTALL ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    SDL_ttf
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
    GIT_TAG        release-2.22.0
    GIT_SHALLOW    TRUE
)
set(SDL2TTF_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    pugixml
    GIT_REPOSITORY https://github.com/zeux/pugixml.git
    GIT_TAG        v1.14
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(SDL2 SDL_ttf pugixml)

# ── stb_image (download if missing) ─────────────────────────────
if(NOT EXISTS ${NUI_ROOT}/renderer/stb_image.h)
    file(DOWNLOAD
        https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
        ${NUI_ROOT}/renderer/stb_image.h
    )
endif()

# ── Sources ──────────────────────────────────────────────────────
set(EXAMPLE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp

    # Library sources
    ${NUI_ROOT}/core/application.cpp
    ${NUI_ROOT}/core/input.cpp
    ${NUI_ROOT}/renderer/canvas.cpp
    ${NUI_ROOT}/renderer/texture.cpp
    ${NUI_ROOT}/renderer/font.cpp
    ${NUI_ROOT}/renderer/resource.cpp
    ${NUI_ROOT}/../src/generated/embedded_resources.cpp
    ${NUI_ROOT}/ui/widget.cpp
    ${NUI_ROOT}/ui/label.cpp
    ${NUI_ROOT}/ui/button.cpp
    ${NUI_ROOT}/ui/image.cpp
    ${NUI_ROOT}/ui/editbox.cpp
    ${NUI_ROOT}/ui/progressbar.cpp
    ${NUI_ROOT}/ui/scrollview.cpp
    ${NUI_ROOT}/xml/layout_loader.cpp
)

add_executable(${PROJECT_NAME} ${EXAMPLE_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${NUI_ROOT}
    ${NUI_ROOT}/../src/generated
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL2::SDL2main
    SDL2::SDL2-static
    SDL2_ttf::SDL2_ttf-static
    pugixml
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE imm32 version winmm setupapi)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE dl pthread m)
endif()

# Copy library resources next to the binary
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/../../resources
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
    COMMENT "Copying library resources"
)

# Also copy example-specific layout.xml into resources/layouts/
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/layout.xml
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources/layouts/example.xml
    COMMENT "Copying example layout"
)
