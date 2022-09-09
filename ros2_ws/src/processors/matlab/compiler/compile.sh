#!/bin/bash
find . ! (-name 'MatlabProcessorInterface.h' -o -name 'MatlabProcessorInterface.cpp') -type f \( -iname \*.h -o -iname \*.cpp \)  -exec cat {} +

 rm -f {} +

cp ../codegen/lib/run_processor/MatlabProcessorInterface.cpp .
cp ../codegen/lib/run_processor/MatlabProcessorInterface.h .
cp ../codegen/lib/run_processor/*.h .
cp ../codegen/lib/run_processor/*.cpp .

PROCESSOR_H=MatlabProcessorInterface.h
sed -i -e 's/MatlabProcessorInterface \*init/virtual MatlabProcessorInterface \*init/' $PROCESSOR_H
sed -i -e 's/void/virtual void/' $PROCESSOR_H

cmake .
make all
