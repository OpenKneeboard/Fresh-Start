add_executable(
  main
  WIN32
  main.cpp
  Artifact.hpp
  Version.hpp
  Versions.hpp
  artifacts/HKCULayer.cpp
  artifacts/HKCULayer.hpp
  artifacts/HKLMLayer.cpp
  artifacts/HKLMLayer.hpp
  artifacts/MSIXInstallation.cpp
  artifacts/MSIXInstallation.hpp
  artifacts/ProgramData.cpp
  artifacts/ProgramData.hpp
)
set_target_properties(
  main
  PROPERTIES
  OUTPUT_NAME "OpenKneeboard-Removal-Tool"
)
target_include_directories(main PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

set(fredemmott-gui_SOURCE_DIR "" CACHE PATH "Path to a local checkout of fredemmott-gui")
if (fredemmott-gui_SOURCE_DIR)
  set(ENABLE_IMPLICIT_BACKENDS OFF)
  set(ENABLE_DIRECT2D ON)
  add_subdirectory(
    "${fredemmott-gui_SOURCE_DIR}"
    "${CMAKE_BINARY_DIR}/third-party/fredemmott-gui"
    EXCLUDE_FROM_ALL
  )
else ()
  find_package(fredemmott-gui CONFIG REQUIRED)
endif ()
target_link_libraries(main PRIVATE fredemmott-gui::fredemmott-gui)