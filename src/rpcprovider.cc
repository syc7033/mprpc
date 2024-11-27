#include "rpcprovider.h"
#include "mprpcapplication.h"
#include "rpcheader.pb.h"
#include "logger.h"
#include "zookeeperutil.h"

/**
 * json和protobuf 数据传输协议
 * json : 存储的是key-value(键值对)、字符流、相同带宽数据传输量小
 * protobuf : 是紧密存储的(不携带除数据以后其他信息)、字节流、相同数据传传输量大
 * protobuf 提供了 rpc服务 通过protoc 可以生成对 服务对象的抽象service 以及对 服务方法的抽象 methodDesc
 */


/**
 *                                                        => method方法对象
 * service_name => service 描述 => service* 记录服务对象   => method方法对象
 *                                                       => method方法对象
 */


// 本地服务 => rpc服务 => .proto文件 rpc方法的参数 返回值 以及方法描述 然后通过protoc 编译生成 对应cpp文件
// 
// 这里是框架提供给外部使用的，可以发布rpc方法的函数接口
void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    ServiceInfo service_info;
    // 获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor *pServiceDesc = service->GetDescriptor();
    
    // 获取服务的名字
    std::string service_name = pServiceDesc->name();
     
    // 获取服务对象service的方法的数量
    int methodCnt = pServiceDesc->method_count();

    // std::cout << "service_name: " << service_name << std::endl;
    LOG_INFO("service_name:%s", service_name.c_str());

    for(int i = 0; i < methodCnt; i++)
    {
        // 获取了服务对象指定下标服务方法的描述(抽象描述) UserService Login
        const google::protobuf::MethodDescriptor* pmethodDesc =  pServiceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        service_info.m_methodMap.insert({method_name, pmethodDesc});

        // std::cout << "method_name: " << method_name << std::endl;
        LOG_INFO("method_name: %s", method_name.c_str());
    }
    service_info.m_service = service;
    m_serviceMap.insert({service_name, service_info});
}

// 启动rpc服务结点，开启提供rpc远程网络调度服务
void RpcProvider::Run()
{
    std::string ip = MprpcApplication::getInstance().GetConfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::getInstance().GetConfig().Load("rpcserverport").c_str());

    muduo::net::InetAddress address(ip, port);

    // 创建muduo库提供的TcpServer服务器对象
    muduo::net::TcpServer server(&m_eventLoop, address, "RpcProvider");
    // 绑定新用户的连接回调，和已连接用户的读写事件回调方法 网络模块和业务模块分离
    server.setConnectionCallback(std::bind(&RpcProvider::onConnection, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::onMessage, this,
                            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    // 设置muduo库的线程数量
    server.setThreadNum(4);

    // 把当前rpc节点上要发布的服务全部注册到zk上面，让rpc client可以从zk上发现服务
    // session timeout   30s     zkclient 网络I/O线程  1/3 * timeout 时间发送ping消息
    ZkClient zkCli;
    zkCli.Start();
    // service_name为永久性节点    method_name为临时性节点
    for (auto &sp : m_serviceMap) 
    {
        // /service_name   /UserServiceRpc
        std::string service_path = "/" + sp.first;
        zkCli.Create(service_path.c_str(), nullptr, 0);
        for (auto &mp : sp.second.m_methodMap)
        {
            // /service_name/method_name   /UserServiceRpc/Login 存储当前这个rpc服务节点主机的ip和port
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);
            // ZOO_EPHEMERAL表示znode是一个临时性节点
            zkCli.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }
    }

    // std::cout << "RpcProvider start service at ip:" << ip << " port: " << port << std::endl;
    LOG_INFO("RpcProvider start service at ip: %s port: %d", ip.c_str(), port);
    // 启动网络服务
    server.start();
    m_eventLoop.loop();
}

// 新用户连接 调用的回调函数
void RpcProvider::onConnection(const muduo::net::TcpConnectionPtr& conn)
{
    if(!conn->connected())
    {
        // 和rpc Client的连接断开了
         conn->shutdown();
    }
}


/**
 * 在框架内部, RpcProvider 和 rpcConsumer 协商好之间通信用的protobuf数据类型
 * head_str: service_name method_name args_size
 * args_str: args 定义proto的message类型 进行数据头的序列化和反序列化
 * 
 * header_size(4B) + head_str + args_str
 * 
 * std::string insert和copy方法
 */

// 当已连接用户 发生读写事件需要调用的回调函数
void RpcProvider::onMessage(const muduo::net::TcpConnectionPtr &conn, 
                            muduo::net::Buffer *buffer, 
                            muduo::Timestamp time)
{
    // 网络上接收的远程rpc调用请求的字符流    Login args
    std::string recv_buf = buffer->retrieveAllAsString();

    // 从字符流中读取前4个字节的内容
    uint32_t header_size = 0;
    recv_buf.copy((char*)&header_size, 4, 0);

    // 根据header_size读取数据头的原始字符流，反序列化数据，得到rpc请求的详细信息
    std::string rpc_header_str = recv_buf.substr(4, header_size);
    mprpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if (rpcHeader.ParseFromString(rpc_header_str))
    {
        // 数据头反序列化成功
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.agrs_size();
    }
    else
    {
        // 数据头反序列化失败
        // std::cout << "rpc_header_str:" << rpc_header_str << " parse error!" << std::endl;
        LOG_ERR("rpc_header_str: %s parse error!", rpc_header_str.c_str());
        return;
    }

    // 获取rpc方法参数的字符流数据
    std::string args_str = recv_buf.substr(4 + header_size, args_size);

    // 打印调试信息
    LOG_INFO("============================================");
    LOG_INFO("header_size: %d", header_size);
    LOG_INFO("rpc_header_str: %s", rpc_header_str.c_str());
    LOG_INFO("service_name: %s", service_name.c_str());
    LOG_INFO("method_name: %s", method_name.c_str());
    LOG_INFO("args_str: %s", args_str.c_str());
    LOG_INFO("============================================");
    // std::cout << "============================================" << std::endl;
    // std::cout << "header_size: " << header_size << std::endl; 
    // std::cout << "rpc_header_str: " << rpc_header_str << std::endl; 
    // std::cout << "service_name: " << service_name << std::endl; 
    // std::cout << "method_name: " << method_name << std::endl; 
    // std::cout << "args_str: " << args_str << std::endl; 
    // std::cout << "============================================" << std::endl;

    // 获取service对象和method对象
    auto it = m_serviceMap.find(service_name);
    if (it == m_serviceMap.end())
    {
        // std::cout << service_name << " is not exist!" << std::endl;
        LOG_ERR("%s is not exist!", service_name.c_str());
        return;
    }

    auto mit = it->second.m_methodMap.find(method_name);
    if (mit == it->second.m_methodMap.end())
    {
        // std::cout << service_name << ":" << method_name << " is not exist!" << std::endl;
        LOG_ERR("%s : %s is not exists!", service_name.c_str(), method_name.c_str());
        return;
    }

    google::protobuf::Service *service = it->second.m_service; // 获取service对象  new UserService
    const google::protobuf::MethodDescriptor *method = mit->second; // 获取method对象  Login

    // 生成rpc方法调用的请求request和响应response参数
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if (!request->ParseFromString(args_str))
    {
        std::cout << "request parse error, content:" << args_str << std::endl;
        LOG_ERR("request parse error, content: %s", args_str.c_str());
        return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();


    // 给下面的method方法的调用，绑定一个Closure的回调函数
    google::protobuf::Closure *done = google::protobuf::NewCallback<RpcProvider, 
                                                                    const muduo::net::TcpConnectionPtr&, 
                                                                    google::protobuf::Message*>
                                                                    (this, 
                                                                    &RpcProvider::SendRpcResponse, 
                                                                    conn, response);

    // 在框架上根据远端rpc请求，调用当前rpc节点上发布的方法
    // new UserService().Login(controller, request, response, done)
    service->CallMethod(method, nullptr, request, response, done);
}


// Closure的回调操作，用于序列化rpc的响应和网络发送
void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr& conn, google::protobuf::Message* reponse)
{
    std::string response_str;
    if(reponse->SerializeToString(&response_str)) // response 进行序列化
    {
        // 序列化后 直接发送给对端
        conn->send(response_str);
    }
    else
    {
        // std::cout << "serialize response_str error!" << std::endl;
        LOG_ERR("serialize response_str error!");
    }
    conn->shutdown(); // 模拟http的短链接服务，右RPC服务的提供方主动断开连接
}

/** rpcProvider rpc服务的提供方主要干了三件事
 * 1.借助muduo库，实现网络消息的收发，然后借助protobuf进行数据的序列化和反序列化
 * 2.rpcProvider提供了一个接口，通过这个接口我们可以把rpc服务和rpc方法注册到rpcProvider中
 * 3.onMessage当接收到rpc请求的时，自动调用业务层的rpc方法(按照约定好的方式解析service_name和method_name，利用protobuf提供的抽象类Service、MethodDesc，调用rpc方法，并传入相应的请求、响应、回调)
 * 
 */