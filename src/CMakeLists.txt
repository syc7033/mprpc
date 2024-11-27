# 当前路径上所有源文件名字 都放入 SRC_LIST中
aux_source_directory(. SRC_LIST)

add_library(mprpc ${SRC_LIST})

target_link_libraries(mprpc muduo_net muduo_base pthread zookeeper_mt)