cmake_minimum_required(VERSION 3.8)
project(trigger_processor)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(mtms_eeg_interfaces REQUIRED)
find_package(mtms_trigger_interfaces REQUIRED)

# Install trigger_processor executable.

add_executable(trigger_processor
  src/trigger_processor.cpp
)

target_include_directories(trigger_processor
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

ament_target_dependencies(
  trigger_processor
  rclcpp
  std_msgs
  mtms_eeg_interfaces
  mtms_trigger_interfaces
)

install(TARGETS
  trigger_processor
  DESTINATION lib/${PROJECT_NAME})

# Install the launch directory.
install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME}/
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
