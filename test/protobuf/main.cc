#include "test.pb.h"
#include <iostream>
#include <string>

using namespace fixbug;

void test01()
{
    // 定义请求对象
    LoginRequest req;
    req.set_name("施易辰");
    req.set_pwd("123456");
    
    std::string send_str;

    // 请求对象 序列化 字符流
    if(req.SerializeToString(&send_str))
    {
        std::cout << send_str << std::endl;
    }
    
    // 字符流 反序列化 请求对象
    LoginRequest req2;
    if(req2.ParseFromString(send_str))
    {
        std::cout << req2.name() << std::endl;
        std::cout << req2.pwd() << std::endl;        
    }
}

void test02()
{
    LoginResponse rsp;
    // 对于消息类型中嵌套定义其他消息类型, 想要设置值需要用mutable
    ResultCode *rc = rsp.mutable_result();
    rc->set_errcode(1);
    rc->set_errmsg("登录处理失败");
    rsp.set_success(false);
    std::string str;
    // 序列化
    rsp.SerializeToString(&str);

    LoginResponse rsp2;
    // 反序列化
    rsp2.ParseFromString(str);
}

void test03()
{
    GetFriendListRequest req;
    req.set_userid(27);
    std::string str;
    req.SerializeToString(&str);

    GetFriendListResponse rsp;
    // 如果对象的成员变量又是一个对象，则通过mutable_result获取他的指针
    ResultCode *rc = rsp.mutable_result();
    rc->set_errcode(0);
    rc->set_errmsg("无错误");

    User *user1 = rsp.add_friend_list();
    user1->set_name("zhang san");
    user1->set_age(20);
    user1->set_sex(User::MAN);

    User *user2 = rsp.add_friend_list();
    user2->set_name("li si");
    user2->set_age(20);
    user2->set_sex(User::MAN);

    std::cout << "好友列表个数：" << rsp.friend_list_size() << std::endl;

    int n = rsp.friend_list_size();
    for(int i = 0; i < n; i++)
    {
        User user = rsp.friend_list(i);
        std::cout << user.name() << ' ' << user.age() << ' ' << user.sex() << std::endl;
    }
}
/** 使用protobuf设置成员变量 规律总结
 * 1.对于普通类型的成员变量 直接调用对象的set_member方法设置
 * 2.对于类成员变量，无法直接通过set_member方法设置，需要先通过mutable_member方法获取该类成员的指针
 * 然后再通过该指针去set_member方法设置
 * 3.对于列表类型的成员变量，需要通过add_member方法或者一个指针，然后去设置
 */

int main()
{
    test03();
    return 0;
}