cmake_minimum_required(VERSION 3.8)
project(efield)

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif ()
set(CMAKE_CUDA_STANDARD 17)

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(shape_msgs REQUIRED)
find_package(neuronavigation_interfaces REQUIRED)

set(CMAKE_INSTALL_RPATH "${CMAKE_CURRENT_SOURCE_DIR}/efield_libraries")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


add_executable(efield
        src/efield.cpp
        src/efield_estimation.h
        src/EfieldInterface.h
        )

target_include_directories(efield
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/efield_libraries/include>
        $<INSTALL_INTERFACE:include>
       )

add_library(libefield_estimation SHARED IMPORTED)
set_target_properties(libefield_estimation PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/efield_libraries/libefield_estimation.so)
target_link_libraries(efield libefield_estimation)

ament_target_dependencies(efield rclcpp std_msgs neuronavigation_interfaces shape_msgs)

install(TARGETS
        efield
        DESTINATION lib/${PROJECT_NAME}
        )

install(
        DIRECTORY launch
        DESTINATION share/${PROJECT_NAME}
)

if (BUILD_TESTING)
    find_package(ament_lint_auto REQUIRED)
    ament_lint_auto_find_test_dependencies()
endif ()

ament_package()
