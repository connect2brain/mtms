cmake_minimum_required(VERSION 3.26)
project(efield_estimation VERSION 0.1.0)

include_directories(include)
include_directories(lib)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2 -mfma")

include_directories(include)
include_directories(lib)

find_package(Eigen3 REQUIRED)
find_package(CUDAToolkit)

include_directories(${EIGEN3_INCLUDE_DIR})

set(libtms_LIBRARY ${PROJECT_SOURCE_DIR}/lib/libtms_cpp.so)

add_library(libtms SHARED IMPORTED GLOBAL)
add_library(libtms_static STATIC IMPORTED GLOBAL)
set_property(TARGET libtms_static PROPERTY IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/libtms_cpp_static.a)


set_target_properties(libtms PROPERTIES IMPORTED_LOCATION ${libtms_LIBRARY})


add_library(efield_estimation SHARED
        src/efield_estimation.cpp
        )

target_link_libraries(efield_estimation PUBLIC libtms libtms_static Eigen3::Eigen CUDA::cudart)

list(APPEND PACKAGE_LIBRARIES ${PROJECT_NAME})

# Mark executables and/or libraries for installation
install(TARGETS ${PACKAGE_LIBRARIES} EXPORT ${PROJECT_NAME}-targets DESTINATION lib)
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


