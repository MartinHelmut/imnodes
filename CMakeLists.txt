cmake_minimum_required(VERSION 3.22)

set(IMNODES_VERSION_MAJOR 1)
set(IMNODES_VERSION_MINOR 0)
set(IMNODES_VERSION_PATCH 0)
set(IMNODES_VERSION ${IMNODES_VERSION_MAJOR}.${IMNODES_VERSION_MINOR}.${IMNODES_VERSION_PATCH})

project(
  imnodes
  DESCRIPTION "A small, dependency-free node editor for Dear ImGUI."
  VERSION ${IMNODES_VERSION}
  LANGUAGES CXX)

# Options

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(IMNODES_INSTALL "Generate install target" OFF)
option(IMNODES_BUILD_SHARED "Build imnodes as shared library" OFF)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif ()

# Check if project is source root
if (NOT DEFINED IMNODES_ROOT_PROJECT)
  if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(IMNODES_ROOT_PROJECT ON)
  else ()
    set(IMNODES_ROOT_PROJECT OFF)
  endif ()
endif ()

if (IMNODES_ROOT_PROJECT)
  message(STATUS "Fetching imgui ...")

  include(FetchContent)

  FetchContent_Declare(
    imgui
    GIT_REPOSITORY "https://github.com/ocornut/imgui.git"
    GIT_TAG master
  )

  FetchContent_GetProperties(imgui)
  if (NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
  endif ()

  add_library(imgui
    ${imgui_SOURCE_DIR}/imgui.cpp ${imgui_SOURCE_DIR}/imgui.h
    ${imgui_SOURCE_DIR}/imconfig.h ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp ${imgui_SOURCE_DIR}/imgui_internal.h
    ${imgui_SOURCE_DIR}/imgui_tables.cpp ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imstb_rectpack.h ${imgui_SOURCE_DIR}/imstb_textedit.h
    ${imgui_SOURCE_DIR}/imstb_truetype.h)

  add_library(imgui::imgui ALIAS imgui)
  target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})
  FetchContent_MakeAvailable(imgui)
endif ()

# Library setup

if (IMNODES_BUILD_SHARED OR BUILD_SHARED_LIBS)
  message(STATUS "Build ${PROJECT_NAME} as shared library")
  set(IMNODES_LIBRARY_TYPE SHARED)
else ()
  message(STATUS "Build ${PROJECT_NAME} as static library")
  set(IMNODES_LIBRARY_TYPE STATIC)
endif ()

add_library(${PROJECT_NAME} ${IMNODES_LIBRARY_TYPE}
  src/imnodes.cpp src/imnodes.hpp src/imnodes_internal.hpp)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
  VERSION ${IMNODES_VERSION}
  SOVERSION ${IMNODES_VERSION_MAJOR})

target_include_directories(${PROJECT_NAME} PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

target_link_libraries(${PROJECT_NAME} PUBLIC imgui::imgui)

# Compile options

if (MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else ()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif ()

# Installation

if (IMNODES_INSTALL)
  message(STATUS "Install ${PROJECT_NAME}")
  install(TARGETS ${PROJECT_NAME})
endif ()
