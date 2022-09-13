#!/bin/bash
cmake -S . -B build -G "Xcode"
cmake --build build --config "Debug"