#!/bin/bash
echo "building dino..."

# remove -O3 flag for slightly faster compile times
sudo clang++ -O3 -Wno-unknown-warning-option `llvm-config --cxxflags --ldflags --system-libs --libs core` -fcxx-exceptions src/*.cpp src/*/*.cpp src/*/*/*.cpp -o /usr/local/bin/dino

# clang++ -g -O3 -Wno-unknown-warning-option `llvm-config --cxxflags --ldflags --system-libs --libs core` -fcxx-exceptions src/*.cpp src/*/*.cpp src/*/*/*.cpp -o dino

if [ $? -ne 0 ]
then
    echo "build failed!"
else
    echo "build succeeded!"
    # sudo cp dino /usr/local/bin
fi
