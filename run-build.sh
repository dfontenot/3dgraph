#!/bin/bash

# source: https://github.com/conan-io/examples2/blob/main/tutorial/consuming_packages/simple_cmake_project/run_example.sh
set -e
set -x

BASEDIR=$(dirname "$0")
pushd "$BASEDIR"

cmake_timestamp=$(date +%s -r CMakeLists.txt)

if [ -f build/CmakeCache.txt ]; then
  cache_timestamp=$(date +%s -r build/CMakeCache.txt)

  # not always necessary, but for local dev ensure clean cache
  if [ "$cmake_timestamp" -gt "$cache_timestamp" ]; then
    echo "clearing cmake cache"
    rm -rf build
  fi
fi

mkdir -p build

conan install . --output-folder=build --build=missing

cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build .

popd
