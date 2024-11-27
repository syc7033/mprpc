
# set(SRC_LIST calluserservice.cc ../user.pb.cc)
set(SRC_LIST callfriendservice.cc ../friend.pb.cc)

add_executable(consumer ${SRC_LIST})

# 编译连接的库文件
target_link_libraries(consumer mprpc protobuf)

