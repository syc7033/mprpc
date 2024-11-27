#include <iostream>
#include "friend.pb.h"
#include "mprpcapplication.h"



int main(int argc, char **argv)
{
    // 整个程序启动以后，想使用mrpc框架享受rpc调用服务，就必须调用框架的初始化函数
    MprpcApplication::Init(argc, argv);

    // 演示调用远程发布的rpc方法Login
    fixbug::FriendServiceRpc_Stub stub(new MprpcChannel());

    // rpc方法的请求参数
    fixbug::GetFriendListRequest request;
    request.set_userid(27);

    // rpc方法的响应
    fixbug::GetFriendListResponse response;

    MprpcController controller;
    // 发起rpc方法的调用 同步rpc调用过程 MprpcChannel::callMethod方法
    stub.GetFriendList(&controller, &request, &response, nullptr); // RpcChannel->RpcChannel::callMethod 集中来做所有rpc方法调用的序列化和网络发送
    
    // 一次rpc调用完成，读取响应
    if(controller.Failed())
    {
        std::cout << controller.ErrorText() << std::endl;
    }
    else
    {
        if(response.result().errcode() == 0)
        {
            std::cout << "rpc GetFriendList response success!" << std::endl;
            int size = response.friends_size();
            for(int i = 0; i < size; i++)
            {
                std::cout << "index: " << (i + 1) << "name: " << response.friends(i) << std::endl;
            }
        }
        else
        {
            std::cout << "rpc login reponse error: " << response.result().errmsg() << std::endl;
        }
    }
    return 0;
}