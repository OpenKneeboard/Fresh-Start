set(CONFIG_HPP "${CMAKE_CURRENT_BINARY_DIR}/include/config.hpp")
block()
  if (${CMAKE_PROJECT_VERSION_TWEAK})
    set(PROJECT_READABLE_VERSION "${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH}+${VERSION_TWEAK_PREFIX}${VERSION_TWEAK}")
  else ()
    set(PROJECT_READABLE_VERSION "${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH}+local.0")
  endif ()
  configure_file(
    "config.hpp.in"
    "${CONFIG_HPP}"
    @ONLY
  )
endblock()

add_executable(
  main
  WIN32
  "${CONFIG_HPP}"
  main.cpp
  Artifact.hpp
  Version.hpp
  Versions.hpp
  artifacts/BasicMSIArtifact.cpp
  artifacts/BasicMSIArtifact.hpp
  artifacts/HKCULayer.cpp
  artifacts/HKCULayer.hpp
  artifacts/HKLMLayer.cpp
  artifacts/HKLMLayer.hpp
  artifacts/MSIInstallation.cpp
  artifacts/MSIInstallation.hpp
  artifacts/MSIXInstallation.cpp
  artifacts/MSIXInstallation.hpp
  artifacts/MultipleMSIInstallations.cpp
  artifacts/MultipleMSIInstallations.hpp
  artifacts/ProgramData.cpp
  artifacts/ProgramData.hpp
  artifacts/SavedGamesSettings.cpp
  artifacts/SavedGamesSettings.hpp
)
set_target_properties(
  main
  PROPERTIES
  OUTPUT_NAME "OpenKneeboard-Removal-Tool"
)
target_include_directories(main PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/include")

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