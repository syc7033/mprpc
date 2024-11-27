#include "mprpccontroller.h"

MprpcController::MprpcController()
{
    m_failed = false;
    m_errTest = "";
}

void MprpcController::Reset()
{
    m_failed = false;
    m_errTest = "";
}

bool MprpcController::Failed() const
{
    return m_failed;
}

std::string MprpcController::ErrorText() const
{
    return m_errTest;
}

void MprpcController::SetFailed(const std::string& reason)
{
    m_failed = true;
    m_errTest = reason;
}


// 目前未实现具体的功能
void MprpcController::StartCancel()
{

}

bool MprpcController::IsCanceled() const
{
    return false;
}

void MprpcController::NotifyOnCancel(google::protobuf::Closure* callback)
{

}