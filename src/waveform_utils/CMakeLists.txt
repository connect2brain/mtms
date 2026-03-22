cmake_minimum_required(VERSION 3.8)
project(waveform_utils)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)

find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(mtms_event_interfaces REQUIRED)
find_package(mtms_targeting_interfaces REQUIRED)

find_package(Boost REQUIRED)

# Install get_default_waveform executable.

add_executable(get_default_waveform
  src/get_default_waveform.cpp
)

target_include_directories(get_default_waveform
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

ament_target_dependencies(get_default_waveform rclcpp std_msgs mtms_event_interfaces mtms_targeting_interfaces)

# Install get_multipulse_waveforms executable.

add_executable(get_multipulse_waveforms
  src/get_multipulse_waveforms.cpp
)

target_include_directories(get_multipulse_waveforms
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

ament_target_dependencies(get_multipulse_waveforms rclcpp std_msgs mtms_event_interfaces mtms_targeting_interfaces)

target_link_libraries(get_multipulse_waveforms Boost::boost)

# Install reverse_polarity executable.

add_executable(reverse_polarity
  src/reverse_polarity.cpp
)

target_include_directories(reverse_polarity
PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

ament_target_dependencies(reverse_polarity rclcpp std_msgs mtms_event_interfaces mtms_targeting_interfaces)


install(TARGETS
  get_default_waveform
  get_multipulse_waveforms
  reverse_polarity
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
