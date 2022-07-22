#!/bin/bash
mkdir -p ./build
cd build
cmake .. -DCMAKE_CONFIGURATION_TYPES="Debug;Release"
make clean
make

