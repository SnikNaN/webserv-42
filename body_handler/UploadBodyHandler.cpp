#include "UploadBodyHandler.hpp"

#include <fstream>
#include <exception>
#include <iostream>

UploadBodyHandler::UploadBodyHandler(const std::string& path): m_isDone(false)
{
    m_dataStream.open(path.c_str());
    if (!m_dataStream.is_open())
    {
        throw std::runtime_error("Can not open file <UploadBodyHandler>");
    }
}

void UploadBodyHandler::ProcessData(std::string &str)
{
    if (str.empty())
    {
        m_isDone = true;
        return ;
    }

    m_dataStream << str;

    if (!m_dataStream)
    {
        throw std::runtime_error("Error sending data to Data Stream <UploadBodyHandler>");
    }
}

bool UploadBodyHandler::GetStatus()
{
    return m_isDone;
}

UploadBodyHandler::~UploadBodyHandler(){
    m_dataStream.close();
}
