cmake_minimum_required(VERSION 3.8)
project(triggerer)

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif ()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(std_msgs REQUIRED)
find_package(event_interfaces REQUIRED)
find_package(system_interfaces REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSERIALPORT REQUIRED libserialport)

include_directories(${LIBSERIALPORT_INCLUDE_DIRS})
link_directories(${LIBSERIALPORT_LIBRARY_DIRS})


add_executable(
        triggerer

        src/triggerer.cpp

        src/memory_utils.cpp
        src/scheduling_utils.cpp
)

target_include_directories(triggerer
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib>
        $<INSTALL_INTERFACE:lib>)

target_link_libraries(triggerer ${LIBSERIALPORT_LIBRARIES})

ament_target_dependencies(
  triggerer
  rclcpp
  rclcpp_action
  std_msgs
  event_interfaces
  system_interfaces
)

install(TARGETS
        triggerer
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
