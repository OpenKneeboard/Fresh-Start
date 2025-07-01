add_executable(
  main
  WIN32
  main.cpp
  Artifact.hpp
  Version.hpp
  Versions.hpp
  artifacts/HKCULayer.cpp
  artifacts/HKCULayer.hpp
  artifacts/MSIXInstallation.cpp
  artifacts/MSIXInstallation.hpp
)
set_target_properties(
  main
  PROPERTIES
  OUTPUT_NAME "OpenKneeboard-Removal-Tool"
)
target_include_directories(main PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

find_package(fredemmott-gui CONFIG REQUIRED)
target_link_libraries(main PRIVATE fredemmott-gui::fredemmott-gui)