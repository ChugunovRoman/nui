cmake_minimum_required(VERSION 3.16)
project(nui VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Output directory ──────────────────────────────────────────────
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ── Dependencies (git submodules in Externals/) ─────────────────
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(SDL2_DISABLE_INSTALL ON CACHE BOOL "" FORCE)
set(SDL2_DISABLE_UNINSTALL ON CACHE BOOL "" FORCE)
set(SDL2TTF_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(Externals/SDL2 EXCLUDE_FROM_ALL)
add_subdirectory(Externals/SDL_ttf EXCLUDE_FROM_ALL)
add_subdirectory(Externals/pugixml EXCLUDE_FROM_ALL)

# ── stb_image (header-only, bundled) ────────────────────────────
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/stb_image.h)
    file(DOWNLOAD
        https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/stb_image.h
        STATUS STB_DOWNLOAD_STATUS
    )
    list(GET STB_DOWNLOAD_STATUS 0 STB_STATUS)
    if(NOT STB_STATUS EQUAL 0)
        message(WARNING "Failed to download stb_image.h - image loading will be unavailable")
    endif()
endif()

# ── Embed resources ─────────────────────────────────────────────
set(NUI_RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/examples/app/resources" CACHE PATH "Resource directory to embed")
set(NUI_GENERATED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/generated")

find_package(Python3 COMPONENTS Interpreter QUIET)
if(Python3_FOUND AND EXISTS "${NUI_RESOURCE_DIR}")
    file(MAKE_DIRECTORY "${NUI_GENERATED_DIR}")
    execute_process(
        COMMAND ${Python3_EXECUTABLE}
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/embed_resources.py"
            "${NUI_RESOURCE_DIR}"
            "${NUI_GENERATED_DIR}"
            "resources"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
endif()

# ── Sources ──────────────────────────────────────────────────────
set(NUI_SOURCES
    # Core
    src/core/application.cpp
    src/core/input.cpp
    src/core/async.cpp

    # Renderer
    src/renderer/canvas.cpp
    src/renderer/texture.cpp
    src/renderer/font.cpp
    src/renderer/resource.cpp

    # UI widgets
    src/ui/widget.cpp
    src/ui/label.cpp
    src/ui/button.cpp
    src/ui/image.cpp
    src/ui/editbox.cpp
    src/ui/progressbar.cpp
    src/ui/scrollview.cpp

    # XML layout loader
    src/xml/layout_loader.cpp

    # Generated (embedded resources)
    ${NUI_GENERATED_DIR}/embedded_resources.cpp

    # Entry point
    src/main.cpp
)

# ── Executable ───────────────────────────────────────────────────
add_executable(${PROJECT_NAME} WIN32 ${NUI_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${NUI_GENERATED_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Externals/SDL2/include
    ${CMAKE_CURRENT_SOURCE_DIR}/Externals/SDL_ttf
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL2::SDL2main
    SDL2::SDL2-static
    SDL2_ttf::SDL2_ttf-static
    pugixml
)

# Platform-specific link libraries
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        imm32 version winmm setupapi
    )
elseif(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        dl pthread m
    )
endif()

# ── Example app ────────────────────────────────────────────────
add_subdirectory(examples/app)
