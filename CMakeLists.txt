cmake_minimum_required(VERSION 3.5)
project(mymoduo)


set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

aux_source_directory(. SRC_LIST)

add_library(mymoduo SHARED ${SRC_LIST})