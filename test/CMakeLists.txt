cmake_minimum_required(VERSION 3.15)
project(SKIPLIST)


# SET(testfile functional_test)
SET(testfile stress_test)
file(GLOB SRC ${PROJECT_SOURCE_DIR}/${testfile}.cpp)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

link_libraries(pthread)

add_executable(${testfile} ${SRC})