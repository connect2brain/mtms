#!/bin/bash
find . ! \( -name 'processor_factory.h' -o -name 'processor_factory.cpp' -o -name 'tmwtypes.h' \) -type f \( -iname \*.h -o -iname \*.cpp \) -exec rm -f {} +

# cp ../codegen/lib/run_processor/MatlabProcessorInterface.cpp .
# cp ../codegen/lib/run_processor/MatlabProcessorInterface.h .
cp ../codegen/lib/run_processor/*.h .
cp ../codegen/lib/run_processor/*.cpp .

PROCESSOR_H=MatlabProcessorInterface.h
sed -i -e 's/MatlabProcessorInterface \*init/virtual MatlabProcessorInterface \*init/' $PROCESSOR_H
sed -i -e 's/void/virtual void/' $PROCESSOR_H

cmake .
make all
