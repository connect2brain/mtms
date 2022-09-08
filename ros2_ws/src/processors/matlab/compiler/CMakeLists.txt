cmake_minimum_required(VERSION 3.8)
project(processor_factory VERSION 0.0.1 DESCRIPTION "testing")
set(CMAKE_CXX_STANDARD 14)


if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif ()

# Declare the library target.
add_library(${PROJECT_NAME} SHARED
        processor_factory.cpp Processor.cpp Processor.h processor_factory.h rtwtypes.h tmwtypes.h coder_array.h
        )

add_executable(processor_factory_exec processor_factory.cpp Processor.cpp Processor.h processor_factory.h rtwtypes.h tmwtypes.h coder_array.h)

# Configure the directories to search for header files.
target_include_directories(${PROJECT_NAME} PRIVATE source)
# Set the version property.
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
# Set the shared object version property to the project's major version.
set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
# Set the public header property to the one with the actual API.
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER processor_factory.h)


# add_executable(processor_factory processor_factory.cpp Processor.cpp Processor.h)

#install(TARGETS
#        processor_factory
#        DESTINATION lib/${PROJECT_NAME}
#        )

#ADD_LIBRARY(processor SHARED ${mylibSRCS})
