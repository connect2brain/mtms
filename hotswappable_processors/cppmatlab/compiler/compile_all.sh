#!/bin/bash

cd .. || exit
matlab -batch "compile"

cd - || exit
bash compile.sh
