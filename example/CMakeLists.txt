cmake_minimum_required(VERSION 3.0)
project(GSECSExample)

set(CMAKE_VERBOSE_MAKEFILE OFF)

set(CMAKE_CXX_FLAGS "-g -std=c++17 -Werror")

set(CMAKE_CXX_FLAGS_DEBUG "-O0")    # 调试包不优化
# set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG " )	# release包优化

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake-build-debug/output)

set(GSECS_ROOT ${CMAKE_CURRENT_LIST_DIR}/..)

#################### 设置头文件目录 ####################

# GSECS对外头文件目录
include_directories(${GSECS_ROOT}/include)

#################### 设置源文件目录 ####################

aux_source_directory(${CMAKE_CURRENT_LIST_DIR} GSECS_EXAMPLE_SRCS)

####################    设置子库    ####################

add_subdirectory(${GSECS_ROOT}/src GSECS)

####################  设置构建目标  ####################

add_executable(GSECSExample ${GSECS_EXAMPLE_SRCS})

####################   设置库文件   ####################

target_link_libraries(GSECSExample GSECS)

include(${GSECS_ROOT}/test/CMakeLists.txt)
