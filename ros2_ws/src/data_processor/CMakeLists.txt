cmake_minimum_required(VERSION 3.8)
project(data_processor)

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif ()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(mtms_interfaces REQUIRED)
find_package(fpga_interfaces REQUIRED)

# MATLAB
find_package(Matlab)
if (Matlab_FOUND)
    message(STATUS "MATLAB FOUND")
    set(Matlab_ROOT_DIR /home/alqio/app/matlab/)
    set(LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:${Matlab_ROOT_DIR}/extern/bin/glnxa65:${Matlab_ROOT_DIR}/sys/os/glnxa64)
    include_directories(${Matlab_ROOT_DIR}/extern/include/)
    link_directories(${Matlab_ROOT_DIR}/extern/bin/glnxa64)
    link_directories(${Matlab_ROOT_DIR}/bin)
    matlab_add_mex(NAME data_processor_matlab SRC
            src/data_processor.cpp
            src/processor.cpp
            src/headers/processor.h
            src/python_processor.cpp
            src/headers/python_processor.h
            src/headers/scheduling_utils.h
            src/headers/scheduling_utils.cpp)

endif ()


# Python
find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})

add_executable(
        data_processor
        src/data_processor.cpp
        src/processor.cpp
        src/headers/processor.h
        src/python_processor.cpp
        src/headers/python_processor.h
        src/headers/scheduling_utils.h
        src/headers/scheduling_utils.cpp
)
ament_target_dependencies(data_processor rclcpp std_msgs mtms_interfaces fpga_interfaces)
ament_target_dependencies(data_processor_matlab rclcpp std_msgs mtms_interfaces fpga_interfaces)

# Python
target_link_libraries(data_processor ${PYTHON_LIBRARIES})

# MATLAB, Linker to libMatlabEngine in link_directories
target_link_libraries(data_processor_matlab MatlabDataArray)
target_link_libraries(data_processor_matlab MatlabEngine)

install(TARGETS
        data_processor
        data_processor_matlab
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
