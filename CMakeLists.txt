cmake_minimum_required(VERSION 3.5)
project(mymoduo)

# 设置库文件的输出路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 设置 C++ 标准为 C++11，并添加调试信息
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")

# 添加头文件搜索路径
include_directories(./include)

# 自动查找 src 目录下的所有源文件
aux_source_directory(./src SRC_LIST)

# 生成可执行文件
# add_executable(echoservertest ${SRC_LIST})
add_library(mymoduo SHARED ${SRC_LIST})