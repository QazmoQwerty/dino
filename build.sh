#!/bin/bash
echo "building dino..."
clang++ -g -O3 -Wno-unknown-warning-option `llvm-config --cxxflags --ldflags --system-libs --libs core` -fcxx-exceptions *.cpp */*.cpp -o dino

if [ $? -ne 0 ]
then
    echo "build failed!"
else
    echo "build succeeded!"
    sudo cp dino /usr/bin
fi