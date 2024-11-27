# 设置cmake最低版本
cmake_minimum_required(VERSION 3.0)

# 项目名
project(mprpc)

# 生成debug版本的可执行文件
set(CMAKE_BUILD_TYPE "Debug")

# 设置可执行文件的输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 设置项目库文件输出路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 设置头文件搜索路径 -I
include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/example)

# 设置连接时候所用的动态库搜索路径 -L
link_directories(${PROJECT_SOURCE_DIR}/lib)

# src里面包含了mprpc框架的所有代码
add_subdirectory(src)
# example包含了mprpc框架使用的实例代码
add_subdirectory(example)