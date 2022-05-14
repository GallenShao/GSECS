#! /bin/bash

cd `dirname $0`/example

if [ ! -d cmake-build-debug ]; then
	mkdir cmake-build-debug
fi

cmake --build cmake-build-debug --target GSECS_test -- -j 9

if [ $? -eq 0 ]; then
  ./cmake-build-debug/output/GSECS_test
fi
