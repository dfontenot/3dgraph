#!/bin/bash

# source: https://github.com/conan-io/examples2/blob/main/tutorial/consuming_packages/simple_cmake_project/run_example.sh
set -e
set -x

BASEDIR=$(dirname "$0")
pushd "$BASEDIR"

rm -rf build

conan install . --output-folder=build --build=missing
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build .
