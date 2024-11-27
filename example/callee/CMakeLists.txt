# set(SRC_LIST userservice.cc ../user.pb.cc)
set(SRC_LIST friendservice.cc ../friend.pb.cc)

add_executable(provider ${SRC_LIST})

# 编译连接的库文件
target_link_libraries(provider mprpc protobuf)

