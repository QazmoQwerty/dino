#!/bin/bash
echo "building dino..."
clang++ -g -O3 -Wno-unknown-warning-option `llvm-config --cxxflags --ldflags --system-libs --libs core` -fcxx-exceptions *.cpp */*.cpp -o dino
sudo cp dino /usr/bin
echo "build complete!"
