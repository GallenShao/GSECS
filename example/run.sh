#! /bin/bash

cd `dirname $0`
./build.sh

if [ $? -eq 0 ]; then
	./cmake-build-debug/output/GSECSExample;
fi;