#!/bin/bash

# 脚本遇到错误立刻退出 不回向下继续执行
set -e

# 把build目录下的文件全部删除
rm -rf `pwd`/build/*
# 进入build目录执行cmake.. make执行
cd `pwd`/build &&
    cmake .. &&
    make
cd ..
# 把头文件导入到lib目录下
cp -r `pwd`/src/include `pwd`/lib