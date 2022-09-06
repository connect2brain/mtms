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

set(MATLAB_FIND_DEBUG true)
# MATLAB
# set(Matlab_ROOT_DIR /home/alqio/app/matlab)
find_package(Matlab)
if (Matlab_FOUND)
    # Following 5 lines taken from https://stackoverflow.com/questions/8880802/cmake-linking-shared-library
    # Fixes runtime error "error while loading shared libraries: libMatlabDataArray.so: cannot open shared object file: No such file or directory"
    SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib64")
    SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib64")

    set(LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:${Matlab_ROOT_DIR}/extern/bin/glnxa64:${Matlab_ROOT_DIR}/sys/os/glnxa64)
    include_directories(${Matlab_ROOT_DIR}/extern/include/)
    link_directories(${Matlab_ROOT_DIR}/extern/bin/glnxa64)
else ()
    message(STATUS "MATLAB NOT FOUND")
endif ()

add_executable(
        data_processor
        src/data_processor.cpp
        src/processor.cpp
        src/headers/processor.h
        src/python_processor.cpp
        src/matlab_processor.cpp
        src/headers/matlab_processor.h
        src/headers/python_processor.h
        src/headers/scheduling_utils.h
        src/headers/scheduling_utils.cpp)

ament_target_dependencies(data_processor rclcpp std_msgs mtms_interfaces fpga_interfaces)

if (Matlab_FOUND)
    # MATLAB, Linker to libMatlabEngine in link_directories
    target_link_libraries(data_processor MatlabDataArray)
    target_link_libraries(data_processor MatlabEngine)
endif ()

# Python
find_package(PythonLibs)
if (PYTHONLIBS_FOUND)
    include_directories(${PYTHON_INCLUDE_DIRS})
    target_link_libraries(data_processor ${PYTHON_LIBRARIES})
endif ()

install(TARGETS
        data_processor
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
