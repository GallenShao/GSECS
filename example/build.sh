#! /bin/bash

cd `dirname $0`

if [ ! -d cmake-build-debug ]; then
	mkdir cmake-build-debug
fi

cmake --build cmake-build-debug --target GSECSExample -- -j 9
