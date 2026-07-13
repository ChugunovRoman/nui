cmake_minimum_required(VERSION 3.16)
project(nui-example LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Static CRT on MSVC
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# ── Locate the library sources ───────────────────────────────────
set(NUI_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../src)
set(NUI_EXTERNALS ${CMAKE_CURRENT_SOURCE_DIR}/../../Externals)

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
    ${NUI_ROOT}/core/async.cpp
    ${NUI_ROOT}/renderer/canvas.cpp
    ${NUI_ROOT}/renderer/texture.cpp
    ${NUI_ROOT}/renderer/font.cpp
    ${NUI_ROOT}/renderer/resource.cpp
    ${NUI_ROOT}/../src/generated/embedded_resources.cpp
    ${NUI_ROOT}/animation/tween.cpp
    ${NUI_ROOT}/animation/animator.cpp
    ${NUI_ROOT}/ui/widget.cpp
    ${NUI_ROOT}/ui/label.cpp
    ${NUI_ROOT}/ui/button.cpp
    ${NUI_ROOT}/ui/image.cpp
    ${NUI_ROOT}/ui/editbox.cpp
    ${NUI_ROOT}/ui/progressbar.cpp
    ${NUI_ROOT}/ui/scrollview.cpp
    ${NUI_ROOT}/ui/slider.cpp
    ${NUI_ROOT}/ui/checkbox.cpp
    ${NUI_ROOT}/ui/radiobutton.cpp
    ${NUI_ROOT}/ui/dropdown.cpp
    ${NUI_ROOT}/ui/tabcontrol.cpp
    ${NUI_ROOT}/ui/treeview.cpp
    ${NUI_ROOT}/ui/tooltip.cpp
    ${NUI_ROOT}/ui/menu.cpp
    ${NUI_ROOT}/ui/dialog.cpp
    ${NUI_ROOT}/xml/layout_loader.cpp
)

add_executable(${PROJECT_NAME} WIN32 ${EXAMPLE_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${NUI_ROOT}
    ${NUI_ROOT}/../src/generated
    ${NUI_EXTERNALS}/SDL2/include
    ${NUI_EXTERNALS}/SDL_ttf
)

# Dependencies are already built by the root CMakeLists.txt
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

# ── Copy runtime resources next to the executable ──────────────────
# Layout + bundled fonts/images live under examples/app/example.xml and
# the top-level resources/ folder. They are needed at runtime because the
# loader reads them from disk relative to the working directory.
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/../../resources"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources"
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_SOURCE_DIR}/example.xml"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources/layouts/example.xml"
    COMMENT "Copying resources and example.xml to output directory"
)
