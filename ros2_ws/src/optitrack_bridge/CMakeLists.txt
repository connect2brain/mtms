cmake_minimum_required(VERSION 3.16)
project(optitrack_bridge)

set(CMAKE_CXX_STANDARD 14)


if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif ()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(neuronavigation_interfaces REQUIRED)

add_executable(
        optitrack_bridge

        src/optitrack_bridge.h
        src/optitrack_bridge.cpp

        src/optitrack_client.h
        src/optitrack_client.cpp
)

target_include_directories(optitrack_bridge
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

add_library(libNatNet SHARED IMPORTED GLOBAL)
set_target_properties(libNatNet PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/libNatNet.so)
target_link_libraries(optitrack_bridge libNatNet)


ament_target_dependencies(optitrack_bridge rclcpp std_msgs neuronavigation_interfaces)

install(TARGETS
        optitrack_bridge
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
message("TARGET: ${CMAKE_BUILD_TYPE}")


include_directories(include)
include_directories(lib)
include_directories(src)
