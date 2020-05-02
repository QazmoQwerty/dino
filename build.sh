#!/bin/bash
echo "building dino..."

# remove -O3 flag for slightly faster compile times + better debug info
sudo clang++ -g -O0 -Wno-unknown-warning-option `llvm-config --cxxflags --ldflags --system-libs --libs core` -fcxx-exceptions src/*.cpp src/*/*.cpp src/*/*/*.cpp -o /usr/local/bin/dino


if [ $? -ne 0 ]
then
    echo "build failed!"
else
    echo "build succeeded!"
    sudo cp /usr/local/bin/dino . # copy the binary into the current folder for debugging
    # sudo cp dino /usr/local/bin
fi
