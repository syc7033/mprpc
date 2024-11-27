#include <iostream>
#include <string>
#include "friend.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"

class FriendService : public fixbug::FriendServiceRpc
{
public:
    // 本地服务
    std::vector<std::string> GetFriendList(uint32_t userid)
    {
        std::cout << "do GetFriendList service! userid: " << userid << std::endl;
        std::vector<std::string> vec;
        vec.push_back("syc");
        vec.push_back("hf");
        vec.push_back("cxl");
        vec.push_back("lh");
        return vec;
    }

    // rpc服务
    void GetFriendList(::google::protobuf::RpcController* controller,
                       const ::fixbug::GetFriendListRequest* request,
                       ::fixbug::GetFriendListResponse* response,
                       ::google::protobuf::Closure* done)
    {
        // 1.获取请求中的参数
        google::protobuf::int32 userid = request->userid();
        // 2.做本地服务
        std::vector<std::string> friendList = GetFriendList(userid);
        // 3.设置响应
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        for(std::string& name : friendList)
        {
            std::string* p = response->add_friends();
            *p = name;
        }
        // 4.执行回调
        done->Run();
    }
private:
};


int main(int argc, char **argv)
{
    // 初始化框架
    MprpcApplication::Init(argc, argv);
    // 创建一个rpc网络服务对象 把FriendService对象发布到rpc结点中
    RpcProvider provider;
    provider.NotifyService(new FriendService());
    // 启动一个rpc服务发布节点 Run以后 进程进入阻塞状态 等待rpc远程调用
    provider.Run();
    return 0;
}