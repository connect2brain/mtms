#!/bin/bash

cp ../codegen/lib/run_processor/Processor.cpp .
cp ../codegen/lib/run_processor/Processor.h .

cmake .
make all