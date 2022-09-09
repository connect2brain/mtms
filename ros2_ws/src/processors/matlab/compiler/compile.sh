#!/bin/bash

cp ../codegen/lib/run_processor/MatlabProcessorInterface.cpp .
cp ../codegen/lib/run_processor/MatlabProcessorInterface.h .

PROCESSOR_H=MatlabProcessorInterface.h
sed -i -e 's/MatlabProcessorInterface \*init/virtual MatlabProcessorInterface \*init/' $PROCESSOR_H
sed -i -e 's/void/virtual void/' $PROCESSOR_H

#cmake .
#make all