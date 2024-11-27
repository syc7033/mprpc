#pragma once

#include "google/protobuf/service.h"


#include <memory>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <string>
#include <functional>
#include <google/protobuf/descriptor.h>
#include <unordered_map>



// 框架提供的专门用于服务发布的rpc网络对象类（RPC网络模块）
class RpcProvider
{
public:
    // 这里是框架提供给外部使用的，可以发布rpc方法的函数接口
    void NotifyService(google::protobuf::Service *service);

    // 启动rpc服务结点，开启提供rpc远程网络调度服务
    void Run();

private:
    // 组合的EventLoop
    muduo::net::EventLoop m_eventLoop;

    // service服务类型信息
    struct ServiceInfo
    {
        google::protobuf::Service *m_service; // 保存服务对象
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> m_methodMap; // 保存服务对象的服务方法    
    };
    // 存储注册成功的服务和其服务方法的所有信息 <---抽象层描述--->
    std::unordered_map<std::string, ServiceInfo> m_serviceMap;

    // 新用户连接回调
    void onConnection(const muduo::net::TcpConnectionPtr&);
    // 已连接用户读写事件回调
    void onMessage(const muduo::net::TcpConnectionPtr &conn, 
                            muduo::net::Buffer *buffer, 
                            muduo::Timestamp time);
    // Closure的回调操作，用于序列化rpc的响应和网络发送
    void SendRpcResponse(const muduo::net::TcpConnectionPtr& conn, google::protobuf::Message* response);
};