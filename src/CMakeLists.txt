set(CMAKE_CXX_FLAGS "-g -std=c++17 -Werror")

set(GSECS_ROOT ${CMAKE_CURRENT_LIST_DIR}/..)

#################### 设置头文件目录 ####################

include_directories(${GSECS_ROOT}/include)
include_directories(${GSECS_ROOT}/src)

#################### 设置源文件目录 ####################

aux_source_directory(. GSECS_SRCS)
aux_source_directory(./traverser GSECS_SRCS)

####################  设置构建目标  ####################

add_library(GSECS ${GSECS_SRCS})
