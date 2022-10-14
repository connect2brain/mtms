#!/bin/bash
find . ! \( -name 'processor_factory.h' -o -name 'processor_factory.cpp' -o -name 'tmwtypes.h' -o -name 'matlab_processor_interface.h' -o -name 'matlab_processor_interface.cpp' \) -type f \( -iname \*.h -o -iname \*.cpp -o -iname \*.c \) -exec rm -f {} +
find . \( -name 'cmake_install.cmake' -o -name 'CMakeCache.txt' -o -name 'Makefile' \) -type f -exec rm -f {} +
find . \( -name 'CMakeFiles' \) -type d -exec rm -rf {} +
