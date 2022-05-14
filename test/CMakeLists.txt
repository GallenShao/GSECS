cmake_minimum_required(VERSION 3.14)
project(GSECSTest)

# GoogleTest requires at least C++11
set(CMAKE_CXX_STANDARD 17)

set(GSECS_ROOT ${CMAKE_CURRENT_LIST_DIR}/..)

####################  GoogleTest  ####################

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/e2239ee6043f73722e7aa812a459f54a28552929.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

#################### 设置源文件目录 ####################

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src GSECS_TEST_SRCS)

####################    设置子库    ####################

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_LIST_DIR)
    add_subdirectory(${GSECS_ROOT}/src GSECS)
endif()

####################    开启测试    ####################

enable_testing()

add_executable(
        GSECS_test
        ${CMAKE_CURRENT_LIST_DIR}/main.cpp
        ${GSECS_TEST_SRCS}
)
target_link_libraries(
        GSECS_test
        gtest_main
)

include(GoogleTest)
gtest_discover_tests(GSECS_test)

####################   设置库文件   ####################

target_link_libraries(GSECS_test GSECS)