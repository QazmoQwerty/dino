#!/bin/bash
echo "building dino..."

sudo clang++ -g -O0 -Wno-unknown-warning-option `llvm-config --cxxflags --ldflags --system-libs --libs core` -fcxx-exceptions src/*.cpp src/*/*.cpp src/*/*/*.cpp -o /usr/local/bin/dino


if [ $? -ne 0 ]
then
    echo "build failed!"
else
    echo "build succeeded!"
fi
