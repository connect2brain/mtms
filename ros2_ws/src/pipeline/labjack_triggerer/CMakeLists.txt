cmake_minimum_required(VERSION 3.8)
project(labjack_triggerer)

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif ()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(std_msgs REQUIRED)
find_package(event_interfaces REQUIRED)
find_package(system_interfaces REQUIRED)

include_directories(include include/labjack)

find_library(LABJACKM_LIBRARY NAMES LabJackM)

add_executable(
        labjack_triggerer

        src/labjack_triggerer.cpp

        src/memory_utils.cpp
        src/scheduling_utils.cpp
)

target_include_directories(labjack_triggerer
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib>
        $<INSTALL_INTERFACE:lib>)

target_link_libraries(labjack_triggerer ${LABJACKM_LIBRARY})

ament_target_dependencies(
  labjack_triggerer
  rclcpp
  rclcpp_action
  std_msgs
  event_interfaces
  system_interfaces
)

install(TARGETS
        labjack_triggerer
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
