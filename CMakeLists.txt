cmake_minimum_required(VERSION 3.31)

# Mostly to suppress the 'Manually-specified variables were not used by the project' thing:
if (CMAKE_TOOLCHAIN_FILE)
  message(STATUS "Toolchain: ${CMAKE_TOOLCHAIN_FILE}")
endif ()

include(cmake/hybrid-crt.cmake)

set(VERSION_TWEAK 0 CACHE STRING "Final numeric component of version number")
set(VERSION_TWEAK_PREFIX "" CACHE STRING "String prefix for final component of version number")
project(OKB-Fresh-Start VERSION 0.0.1.${VERSION_TWEAK} LANGUAGES CXX)

if (MSVC)
  # Incremental linking makes rebuilding quicker, but as of 2025-07-12, increases the binary size by ~ 1/3rd
  # Don't do this in release builds.
  #
  # - we generally don't want to rebuild release builds with small changes
  # - smaller binaries are good
  # - as well as the direct increase, incremental linking breaks the SizeBench tool
  add_link_options("$<$<NOT:$<CONFIG:Debug>>:/INCREMENTAL:NO>")
endif ()

if (${CMAKE_PROJECT_VERSION_TWEAK})
  set(PROJECT_READABLE_VERSION "${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH}+${VERSION_TWEAK_PREFIX}${VERSION_TWEAK}")
else ()
  set(PROJECT_READABLE_VERSION "${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH}+local.0")
endif ()
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/version-semver.txt" "${PROJECT_READABLE_VERSION}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/version-quad.txt" "${CMAKE_PROJECT_VERSION}")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options(
  # Standard C++ exception behavior
  "/EHsc"
  # UTF-8 sources
  "/utf-8"
)

option(PERMISSIVE "Disable extra warnings, warning on error, etc" OFF)
if (NOT PERMISSIVE)
  include(cmake/warnings.cmake)
endif ()

add_compile_definitions(
  NOMINMAX
  UNICODE
  _UNICODE
)
set(CMAKE_LINK_LIBRARIES_ONLY_TARGETS ON)

add_subdirectory(src)
