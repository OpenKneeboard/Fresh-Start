cmake_minimum_required(VERSION 3.31)

# Mostly to suppress the 'Manually-specified variables were not used by the project' thing:
if (CMAKE_TOOLCHAIN_FILE)
  message(STATUS "Toolchain: ${CMAKE_TOOLCHAIN_FILE}")
endif ()

include(cmake/hybrid-crt.cmake)

project(OpenKneeboard-Removal-Tool VERSION 0.0.1 LANGUAGES CXX)

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
