#include <iostream>
#include <string>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include "logger.h"
/**
 * UserService原来是一个本地服务，提供了俩个进程内的本地方法，Login和GetFriendLists
 */

// rpc服务发布端（rpc服务提供者）
class UserService : public fixbug::UserServiceRpc
{
public:
    // 本地方法 Login
    bool Login(std::string name, std::string pwd)
    {
        std::cout << "doing local service: login" << std::endl;
        std::cout << "name: " << name << "pwd: " << pwd << std::endl; 
        return true;
    }

    // 本地方法 Register
    bool Register(uint32_t id, std::string name, std::string pwd)
    {
        std::cout << "doing local service: Register" << std::endl;
        std::cout << "id: " << id << "name: " << name << "pwd: " << pwd << std::endl; 
        return true;
    }
    
    // 重写基类UserServiceRpc的虚函数 下面这些方法都是框架直接调用的
    /**
     * 1.caller ==> Login(LoginReq) ==> muduo ==> callee
     * 2.callee ==> Login(LoginReq) ==> 交到下面重写的Login方法上了
     */
    void Login(::google::protobuf::RpcController* controller,
                       const ::fixbug::LoginReq* request,
                       ::fixbug::LoginRsp* response,
                       ::google::protobuf::Closure* done)
    {
        // 1.框架给业务层上报了rpc请求参数 LoginReq 应用层解析请求中的参数做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();

        // 2.做本地业务
        bool login_ret = Login(name, pwd); // 做本地业务
        
        // 3.设置响应 响应中添加错误码、错误消息、返回值 
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_sucess(login_ret);

        // 4.执行回调操作 执行响应对象数据的序列化和网络发送（都是由框架完成的）
        done->Run();
    }

    // rpc服务方法 Registerjjjkl
    void Register(::google::protobuf::RpcController* controller,
                       const ::fixbug::RegisterReq* request,
                       ::fixbug::RegisterRsp* response,
                       ::google::protobuf::Closure* done)
    {
        // 1.框架接收到了rpc请求, 并且反序列化为数据对象, 并上报应用层, 我们把参数取出来
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();
        
        // 2.出入参数 调用本地业务
        bool ret = Register(id, name, pwd);

        // 3.根据本地业务的返回结果 设置数据对象的响应
        fixbug::ResultCode * result = response->mutable_result();
        result->set_errcode(0);
        result->set_errmsg("注册成功");
        response->set_success(true);

        // 4.调用框架提供的回调函数(底层就是把你的response序列化为字符流发送给rpc服务调用端)
        done->Run();
    }
private:
};

int main(int argc, char **argv)
{

    // 调用框架的初始化操作
    MprpcApplication::Init(argc, argv);

    // provider是一个rpc网络服务对象，把UserService对象发布到rpc结点上
    RpcProvider provider;
    provider.NotifyService(new UserService());

    // 启动一个RPC服务发布结点，Run以后，进程进入阻塞状态，等待远程rpc调用请求
    provider.Run();
    return 0;
}