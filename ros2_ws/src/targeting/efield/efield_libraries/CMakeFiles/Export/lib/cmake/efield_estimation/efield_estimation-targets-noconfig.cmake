#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "efield_estimation" for configuration ""
set_property(TARGET efield_estimation APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(efield_estimation PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libefield_estimation.so"
  IMPORTED_SONAME_NOCONFIG "libefield_estimation.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS efield_estimation )
list(APPEND _IMPORT_CHECK_FILES_FOR_efield_estimation "${_IMPORT_PREFIX}/lib/libefield_estimation.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
