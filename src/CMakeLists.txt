set(CONFIG_HPP "${CMAKE_CURRENT_BINARY_DIR}/include/config.hpp")
set(VERSION_RC "${CMAKE_CURRENT_BINARY_DIR}/version.rc")
block()
  configure_file(
    "config.hpp.in"
    "${CONFIG_HPP}"
    @ONLY
  )
  configure_file(
    "version.rc.in"
    "${VERSION_RC}.configured"
    @ONLY
  )
  file(
    GENERATE
    OUTPUT "${VERSION_RC}"
    INPUT "${VERSION_RC}.configured"
  )
endblock()

add_executable(
  main
  WIN32
  "${CONFIG_HPP}"
  "${VERSION_RC}"
  app.exe.manifest
  main.cpp
  Artifact.hpp
  Version.hpp
  Versions.hpp
  artifacts/BasicMSIArtifact.cpp
  artifacts/BasicMSIArtifact.hpp
  artifacts/DCSHooks.cpp
  artifacts/DCSHooks.hpp
  artifacts/FilesystemArtifact.cpp
  artifacts/FilesystemArtifact.hpp
  artifacts/HKCULayer.cpp
  artifacts/HKCULayer.hpp
  artifacts/HKLMLayer.cpp
  artifacts/LocalAppDataSettings.cpp
  artifacts/LocalAppDataSettings.hpp
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
  OUTPUT_NAME "OpenKneeboard-Fresh-Start"
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

if (MSVC)
  target_link_options(
    main
    PRIVATE
    # Incremental linking makes rebuilding quicker, but as of 2025-07-12, increases the binary size by ~ 1/3rd
    # Don't do this in release builds.
    #
    # - we generally don't want to rebuild release builds with small changes
    # - smaller binaries are good
    # - as well as the direct increase, incremental linking breaks the SizeBench tool
    "$<$<NOT:$<CONFIG:Debug>>:/INCREMENTAL:NO>"
    # COMDAT folding; drops off another 1MB
    "$<$<NOT:$<CONFIG:Debug>>:/OPT:ICF>"
    # Remove unused functions and data, in particular, WinRT types
    "$<$<NOT:$<CONFIG:Debug>>:/OPT:REF>"
    # We already specify this via the manifest, but CMake tries to conflict
    "/MANIFESTUAC:level='requireAdministrator'"
  )
endif ()

find_package(compressed-embed CONFIG REQUIRED)
include(CompressedEmbed)
add_compressed_embed_library(
  licenses
  OUTPUT_CPP "${CMAKE_CURRENT_BINARY_DIR}/licenses.cpp"
  OUTPUT_HPP "${CMAKE_CURRENT_BINARY_DIR}/include/licenses.hpp"
  CLASSNAME Licenses
  INPUTS
  Self "${PROJECT_SOURCE_DIR}/LICENSE"
  CompressedEmbed "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share/compressed-embed/copyright"
  FUI "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share/fredemmott-gui/copyright"
  WIL "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share/wil/copyright"
  Yoga "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share/yoga/copyright"
)
target_include_directories(licenses PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/include")
target_link_libraries(main PRIVATE licenses)
