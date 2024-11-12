#!/bin/bash

set -o errexit
set -o nounset
USAGE="Usage: $(basename $0) [-v | --verbose] [ gcc | clang ] [ reset | clean | debug | release | reldebug]"

CMAKE=cmake
BUILD=./build
TYPE=Debug
COMPILER=gcc
CLEAN=
RESET=
VERBOSE=

for arg; do
  case "$arg" in
    --help|-h)    echo $USAGE; exit 0;;
    -v|--verbose) VERBOSE='VERBOSE=1';;
    debug)        TYPE=Debug ;;
    release)      TYPE=Release ;;
    reldebug)     TYPE=RelWithDebInfo ;;
    clang)        COMPILER=clang ;;
    gcc)          COMPILER=gcc ;;
    clean)        CLEAN=1 ;;
    reset)        RESET=1 ;;
    *)            echo -e "unknown option $arg\n$USAGE" >&2;  exit 1 ;;
  esac
done

cd "$(realpath $(dirname $0))"

BUILD_DIR=$BUILD/$COMPILER/$TYPE

if [[ "$COMPILER" == "clang" ]]
then
    COMPILER_OPTIONS="-DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++"
else
    COMPILER_OPTIONS="-DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++"
fi

[[ -n $RESET && -d $BUILD_DIR ]] && rm -rf $BUILD_DIR
    
GENERATOR="$($CMAKE --help | grep '^\*' | awk '{print $2}')"

if [[ "$GENERATOR" == "Ninja" ]]
then
    GENERATOR_OPTIONS=""
  if [[ $VERBOSE != "" ]]
  then
    GENERATOR_OPTIONS="$GENERATOR_OPTIONS -v"
  fi
else
  GENERATOR_OPTIONS="-j8"
  if [[ $VERBOSE != "" ]]
  then
    GENERATOR_OPTIONS="$GENERATOR_OPTIONS VERBOSE=1"
  fi
fi

if command -v ccache > /dev/null 2>&1
then
    CACHE="-DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCMAKE_C_COMPILER_LAUNCHER=ccache"
else
    echo -e '\033[0;31m'"Consider installing \`ccache\` for extra speed!"'\033[0m' 1>&2
    CACHE=""
fi

$CMAKE -S . -B $BUILD_DIR $COMPILER_OPTIONS -DCMAKE_BUILD_TYPE=$TYPE 

[[ -n $CLEAN ]] && $CMAKE --build $BUILD_DIR --target clean

$CMAKE --build $BUILD_DIR -- $GENERATOR_OPTIONS

## copy compile_commands.json for ease
rm -f ./compile_commands.json && ln -s $BUILD_DIR/compile_commands.json .

## run tests
GTEST_COLOR=1 ctest --test-dir $BUILD_DIR/tests --output-on-failure -j