#include <iostream>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "mprpcchannel.h"


int main(int argc, char **argv)
{
    // 整个程序启动以后，想使用mrpc框架享受rpc调用服务，就必须调用框架的初始化函数
    MprpcApplication::Init(argc, argv);

    // 演示调用远程发布的rpc方法Login
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());

    // rpc方法的请求参数
    fixbug::LoginReq request;
    request.set_name("zhang san");
    request.set_pwd("123456");

    // rpc方法的响应
    fixbug::LoginRsp response;

    // 发起rpc方法的调用 同步rpc调用过程 MprpcChannel::callMethod方法
    stub.Login(nullptr, &request, &response, nullptr); // RpcChannel->RpcChannel::callMethod 集中来做所有rpc方法调用的序列化和网络发送
    
    // 一次rpc调用完成，读取响应
    if(response.result().errcode() == 0)
    {
        std::cout << "rpc login reponse: " << response.sucess() << std::endl;
    }
    else
    {
        std::cout << "rpc login reponse error: " << response.result().errmsg() << std::endl;
    }

    // 演示调用远程发布的rpc方法 Register

    // 1.定义rpc方法请求
    fixbug::RegisterReq req;
    req.set_id(27);
    req.set_name("syc");
    req.set_pwd("123456");

    // 2.定义rpc方法响应
    fixbug::RegisterRsp rsp;
    
    // 3.以同步的方法发起rpc调用请求，等待返回结果
    stub.Register(nullptr, &req, &rsp, nullptr);

    // 4.得到rpc方法响应
    // 一次rpc调用完成，读取响应
    if(rsp.result().errcode() == 0)
    {
        std::cout << "rpc register reponse: " << rsp.success() << std::endl;
    }
    else
    {
        std::cout << "rpc register reponse error: " << rsp.result().errmsg() << std::endl;
    }
    return 0;
}