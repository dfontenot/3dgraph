#!/bin/bash

# NOTE: this file is more of a convenience for building locally
# this project will build w/o this script using regular cmake commands

# NOTE: no long arg support in getopts
# and no long arg support for macOS getopt
# manual argument detection for now
NEXT_IS_ARG=0
RUN_TESTS=1
TARGET_REGEX='--target=([[:alnum:]_]+)'
TARGET=
for arg do
  shift
  if [ "$NEXT_IS_ARG" -eq 1 ]; then
    TARGET="--target=${arg}"

    if [[ $arg = "clean" ]]; then
      RUN_TESTS=0
    fi

    NEXT_IS_ARG=0
    continue
  elif [[ "$arg" =~ $TARGET_REGEX ]]; then
    if [[ "${BASH_REMATCH[1]}" = "clean" ]]; then
      RUN_TESTS=0
    fi

    TARGET=$arg
    NEXT_IS_ARG=0
    continue
  elif [[ "$arg" = "--target" ]]; then
    NEXT_IS_ARG=1
    continue
  fi

  set -- "$@" "$arg"
done

# source: https://github.com/conan-io/examples2/blob/main/tutorial/consuming_packages/simple_cmake_project/run_example.sh
set -e
set -x

printf '%s\n' "$@"
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

# TODO: easier and less janky way to do this?
RELEASE_SPECIFIED=
for arg in "$@"; do
  if [[ "$arg" == "-DCMAKE_BUILD_TYPE=Release" ]]; then
    RELEASE_SPECIFIED=Release
  elif [[ "$arg" == "-DCMAKE_BUILD_TYPE=Debug" ]]; then
    RELEASE_SPECIFIED=Debug
  elif [[ "$arg" == "-DCMAKE_BUILD_TYPE=RelWithDebInfo" ]]; then
    RELEASE_SPECIFIED=RelWithDebInfo
  elif [[ "$arg" == "-DCMAKE_BUILD_TYPE=MinSizeRel" ]]; then
    RELEASE_SPECIFIED=MinSizeRel
  fi
done


if [ "${RELEASE_SPECIFIED+set}" = "set" ]; then
  RELEASE_SPECIFIED=Debug
  set -- "$@" "-DCMAKE_BUILD_TYPE=${RELEASE_SPECIFIED}"
fi

conan install . --lockfile=conan.lock --lockfile-partial --lockfile-out=conan.lock --output-folder=build --build=missing -s build_type=$RELEASE_SPECIFIED

cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_POLICY_DEFAULT_CMP0091=NEW $@
cmake --build . $TARGET

if [ $RUN_TESTS -eq 1 ]; then
  ctest --output-on-failure
fi

popd
