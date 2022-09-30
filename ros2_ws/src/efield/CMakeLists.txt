cmake_minimum_required(VERSION 3.8)
project(efield)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(shape_msgs REQUIRED)
find_package(neuronavigation_interfaces REQUIRED)
find_package(efield_estimation REQUIRED)

add_executable(efield_service src/efield_service.cpp)
ament_target_dependencies(efield_service rclcpp std_msgs neuronavigation_interfaces shape_msgs efield_estimation)

target_link_directories(efield_service PUBLIC efield_libraries/lib)
target_include_directories(efield_service PUBLIC efield_libraries/include)

install(TARGETS
  efield_service
  DESTINATION lib/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # uncomment the line when a copyright and license is not present in all source files
  #set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # uncomment the line when this package is not in a git repo
  #set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
