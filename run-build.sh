#!/bin/bash

# source: https://github.com/conan-io/examples2/blob/main/tutorial/consuming_packages/simple_cmake_project/run_example.sh
set -e
set -x

BASEDIR=$(dirname "$0")
pushd "$BASEDIR"

uname_=$(uname)
if [[ $uname_ = "Darwin" ]]; then
  cmake_timestamp=$(stat -f "%B" CMakeLists.txt)
  conanfile_timestamp=$(stat -f "%B" conanfile.txt)
else
  cmake_timestamp=$(date +%s -r CMakeLists.txt)
  conanfile_timestamp=$(date +%s -r conanfile.txt)
fi

if [ -f build/CmakeCache.txt ]; then
  if [[ $uname_ = "Darwin" ]]; then
    cache_timestamp=$(stat -f "%B" build/CMakeCache.txt)
  else
    cache_timestamp=$(date +%s -r build/CMakeCache.txt)
  fi

  # not always necessary, but for local dev ensure clean cache
  if [ "$cmake_timestamp" -gt "$cache_timestamp" ] || [ "$conanfile_timestamp" -gt "$cache_timestamp" ]; then
    # TODO: maybe a cmake --build build --target clean is enough?
    echo "clearing cmake cache"
    rm -rf build
  fi
fi

mkdir -p build

# TODO: unify build type setting across this script and cmakelists.txt
conan install . --output-folder=build --build=missing -s build_type=Debug

cd build
#cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_POLICY_DEFAULT_CMP0091=NEW
# TODO: fix this
#cmake --preset conan-debug ..
cmake --build .
ctest --output-on-failure

popd
