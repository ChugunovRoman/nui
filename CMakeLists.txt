cmake_minimum_required(VERSION 3.16)
project(nui VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Output directory ──────────────────────────────────────────────
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ── Dependencies via FetchContent ────────────────────────────────
include(FetchContent)

# SDL2
FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-2.30.12
    GIT_SHALLOW    TRUE
)
set(SDL2_DISABLE_INSTALL ON CACHE BOOL "" FORCE)
set(SDL2_DISABLE_UNINSTALL ON CACHE BOOL "" FORCE)

# SDL_ttf (wraps FreeType)
FetchContent_Declare(
    SDL_ttf
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
    GIT_TAG        release-2.22.0
    GIT_SHALLOW    TRUE
)
set(SDL2TTF_INSTALL OFF CACHE BOOL "" FORCE)

# pugixml (XML parser)
FetchContent_Declare(
    pugixml
    GIT_REPOSITORY https://github.com/zeux/pugixml.git
    GIT_TAG        v1.14
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(SDL2 SDL_ttf pugixml)

# ── stb_image (header-only, bundled) ────────────────────────────
# Download stb_image.h if not present
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
add_executable(${PROJECT_NAME} ${NUI_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${NUI_GENERATED_DIR}
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

# stb_image implementation (compiled in one .cpp file)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    STB_IMAGE_IMPLEMENTATION_IN_SOURCE
)

# ── Copy resources to build directory ────────────────────────────
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
    COMMENT "Copying resources to output directory"
)

# ── Install ──────────────────────────────────────────────────────
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(DIRECTORY resources/ DESTINATION bin/resources)
