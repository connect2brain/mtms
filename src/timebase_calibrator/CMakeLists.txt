cmake_minimum_required(VERSION 3.8)
project(timebase_calibrator)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(mtms_eeg_interfaces REQUIRED)
find_package(mtms_system_interfaces REQUIRED)
find_package(mtms_event_interfaces REQUIRED)
find_package(mtms_waveform_interfaces REQUIRED)

add_executable(timebase_calibrator
  src/timebase_calibrator.cpp
)

target_include_directories(timebase_calibrator
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

ament_target_dependencies(timebase_calibrator
  rclcpp
  mtms_eeg_interfaces
  mtms_event_interfaces
  mtms_waveform_interfaces
  mtms_system_interfaces)

install(TARGETS
  timebase_calibrator
  DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME}/
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
