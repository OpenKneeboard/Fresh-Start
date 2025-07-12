if (MSVC)
  add_compile_options(
    # Lots of warnings
    "/W4"
    # Error on warning
    "/WX"
  )
  # clang-cl
  if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(
      "-Wno-missing-field-initializers"
      "-Wno-missing-designated-field-initializers"
      "-Wno-sign-compare"
    )
  else ()
    add_compile_options(
      "/wd4244" # using integer literals for floats
      "/wd4250" # inherits via dominance
      "/wd4267" # converting from larger to smaller-sized integers
      "/wd4305" # truncation from 'double' to 'float'
      "/wd4389" # ==/!= between signed and unsigned
      "/wd4702" # unreachable code. Usually complains about std::unreachable() after ifdefs
    )
  endif ()
endif ()
