#!/bin/bash

echo "Compiling MATLAB to C++"
cd .. || exit
matlab -batch "compile"

echo "Compiling C++"
cd - || exit
bash compile.sh
