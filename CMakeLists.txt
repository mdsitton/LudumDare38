####################################################################
#   Project configuration
####################################################################
cmake_minimum_required(VERSION 3.0)

project (PlanetGame)

set(APPLE_APP_NAME "ld38.app")

set(CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE} CACHE STRING "" FORCE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
####################################################################
#   C++14 / C11 support and flags
####################################################################
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 11)
if(MSVC)
    add_definitions("/W4")
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    add_definitions("-Wall")
    # Clean unused sections on link
    add_definitions("-ffunction-sections -fdata-sections -Wl,--gc-sections")
    # Colored output
    add_definitions("-fdiagnostics-color")
endif()

####################################################################
#   Platform detection and rules
####################################################################
set(PL_DEFAULT  0)
set(PL_WINDOWS  1)
set(PL_LINUX    2)
set(PL_OSX      3)

set(OSX_APP_BUNDLE 0)

## This is needed when cross-compiling
list(APPEND CMAKE_FIND_ROOT_PATH "${PROJECT_SOURCE_DIR}/extern")

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    message(STATUS "Building for Windows.")
    set(PLATFORM PL_WINDOWS)

    set(ENV{GLM_ROOT_DIR}    "${PROJECT_SOURCE_DIR}/extern/glm")
    set(ENV{SDL2DIR}         "${PROJECT_SOURCE_DIR}/extern/sdl2")
    set(ENV{FMT_ROOT}    "${PROJECT_SOURCE_DIR}/extern/fmt")


    if(MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244")
    endif()

elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
    message(STATUS "Building for Linux.")
    set(PLATFORM PL_LINUX)

elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    message(STATUS "Building for Mac OS X.")
    set(PLATFORM PL_OSX)

    if(NOT DEFINED ENV{NO_APP})
        set(OSX_APP_BUNDLE 1)
    endif()

else()
    message(STATUS "Platform not detected!")
    set(PLATFORM PL_DEFAULT)
    # Error and abort here ?
endif()


####################################################################
#   Libraries
####################################################################
# The *_ROOT variables specify the location of a local clone
# to be used if a system-wide library is not found

find_package(GLM        REQUIRED)
find_package(OpenGL     REQUIRED)
find_package(SDL2       REQUIRED)
find_package(fmt        REQUIRED)

set(LIBRARIES
    ${CMAKE_DL_LIBS}
    ${OPENGL_LIBRARIES}
    ${SDL2_LIBRARY}
    ${FMT_LIBRARY}
)


# These includes are defined a system headers so any warnings within
# don't get shown during compilation.
include_directories(SYSTEM
    ${CMAKE_SOURCE_DIR}/extern/glad/include
    ${CMAKE_SOURCE_DIR}/extern/spdlog/include
    ${CMAKE_SOURCE_DIR}/extern/stb
    ${GLM_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIR}
    ${FMT_INCLUDE_DIR}
)


####################################################################
#   Source
####################################################################
configure_file (
  "${PROJECT_SOURCE_DIR}/src/config.hpp.in"
  "${PROJECT_SOURCE_DIR}/src/config.hpp"
)

set(CORE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/renderer/batch.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/renderer/renderer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/renderer/shader.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/renderer/texture.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/particles.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/context.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/events.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/keycode.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/parseutils.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/stringutils.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/timing.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/vfs.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/window.hpp
)
set(CORE_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/renderer/batch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/renderer/renderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/renderer/shader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/renderer/texture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/context.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/particles.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/events.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/keycode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/stringutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/timing.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/vfs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/window.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/glad/src/glad.c
)

set(GAME_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/game/game.hpp
)
set(GAME_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/game/game.cpp
)

set(ALL_SOURCE
    ${CORE_SOURCE} ${CORE_HEADERS} ${GAME_SOURCE} ${GAME_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

include_directories(
    ${PROJECT_SOURCE_DIR}/src/core/audio
    ${PROJECT_SOURCE_DIR}/src/core
    ${PROJECT_SOURCE_DIR}/src/core/renderer
    ${PROJECT_SOURCE_DIR}/src/game
    ${PROJECT_SOURCE_DIR}/src
)

# To create tabs in VisualStudio
source_group("src\\core"  FILES ${CORE_SOURCE}  ${CORE_HEADERS})
source_group("src\\game"  FILES ${GAME_SOURCE}  ${GAME_HEADERS})

add_library(ORCore-obj OBJECT ${CORE_SOURCE})

add_executable(planetgame $<TARGET_OBJECTS:ORCore-obj>
                          ${GAME_SOURCE}
                          ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

target_link_libraries(planetgame ${LIBRARIES})
install(TARGETS       planetgame DESTINATION bin)

set(BINARY_DATA_DIR ${CMAKE_BINARY_DIR}/data)
set(SOURCE_DATA_DIR ${CMAKE_SOURCE_DIR}/data)

add_custom_command(TARGET planetgame PRE_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                ${BINARY_DATA_DIR})
add_custom_command(TARGET planetgame PRE_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${SOURCE_DATA_DIR} $<TARGET_FILE_DIR:planetgame>/data/)


if(OSX_APP_BUNDLE)
    set_target_properties(planetgame PROPERTIES MACOSX_BUNDLE TRUE)
endif()
