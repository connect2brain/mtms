cmake_minimum_required(VERSION 3.16)
project(efield_estimation VERSION 0.1.0)

include_directories(include)
include_directories(lib)
set(CMAKE_CXX_STANDARD 14)

set(linalg_LIBRARY ${PROJECT_SOURCE_DIR}/lib/liblinalg.so)
set(phi_LIBRARY ${PROJECT_SOURCE_DIR}/lib/libphi.so)
set(tms_LIBRARY ${PROJECT_SOURCE_DIR}/lib/libtms.so)

add_library(linalg SHARED IMPORTED GLOBAL)
add_library(phi SHARED IMPORTED GLOBAL)
add_library(tms SHARED IMPORTED GLOBAL)

set_target_properties(linalg PROPERTIES IMPORTED_LOCATION ${linalg_LIBRARY})
set_target_properties(phi PROPERTIES IMPORTED_LOCATION ${phi_LIBRARY})
set_target_properties(tms PROPERTIES IMPORTED_LOCATION ${tms_LIBRARY})

add_library(efield_estimation SHARED
        src/efield_estimation.cpp
        include/efield_estimation.h
        )
add_library(EfieldInterface SHARED
        src/EfieldInterface.cpp
        include/EfieldInterface.h
)

target_link_libraries(EfieldInterface linalg phi tms)
target_link_libraries(efield_estimation PUBLIC linalg phi tms EfieldInterface)

list(APPEND PACKAGE_LIBRARIES ${PROJECT_NAME})

# Mark executables and/or libraries for installation
install(TARGETS ${PACKAGE_LIBRARIES} EfieldInterface EXPORT ${PROJECT_NAME}-targets DESTINATION lib)
install(EXPORT ${PROJECT_NAME}-targets DESTINATION lib/cmake/${PROJECT_NAME})
# Mark cpp header files for installation
install(DIRECTORY include
        DESTINATION include)
# Create cmake config files
# See doc: https://cmake.org/cmake/help/latest/module/CMakePackageConfigHelpers.html#example-generating-package-files
include(CMakePackageConfigHelpers)
configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/${PROJECT_NAME}-config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
        INSTALL_DESTINATION lib/cmake/${PROJECT_NAME}
        NO_CHECK_REQUIRED_COMPONENTS_MACRO)

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
        VERSION ${PROJECT_VERSION} COMPATIBILITY ExactVersion)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
        DESTINATION lib/cmake/${PROJECT_NAME})

export(EXPORT ${PROJECT_NAME}-targets FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-targets.cmake)
