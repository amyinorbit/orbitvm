#!/bin/sh
SOURCE_DIR=$1
BUILD_DIR="${SOURCE_DIR}/.build-tests.ignore"

mkdir $BUILD_DIR && cd $BUILD_DIR
cmake $SOURCE_DIR -DCMAKE_BUILD_TYPE=Release
make all > /dev/null
make test
RESULT=$?

[ $RESULT -ne 0 ] && exit 1 
exit 0
