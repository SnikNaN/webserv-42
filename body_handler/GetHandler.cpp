#include "GetHandler.hpp"
#include "../parsers/HttpReqHeader.hpp"

#include <fstream>

namespace ft {

GetHandler::GetHandler(const struct request_headers& reqHeader)
{
    m_reqHeaders = reqHeader;
    m_stream.open("/home/serg/projects/webserv/index.html");
    if (!m_stream.is_open())
    {
        throw std::runtime_error("File can not be open");
    }
}

void GetHandler::ProcessData(std::string &str)
{
    throw std::runtime_error("Not implemented");
}

bool GetHandler::GetData(std::string &str)
{
    // std::cout << "HEADER APPENDING" << std::endl;

    str.append(HEADER_OK);
    
    // std::cout << "HEADER APPENDED" << std::endl;
     if (!(m_stream >> str))
     {
        std::cerr << "Error reading file" << std::endl;
        std::runtime_error("Error reading file");
     }

     if (m_stream.eof())
     {
        m_isDone = true;
        m_stream.close();
     }
     return true;
}

bool GetHandler::GetStatus()
{
    return m_isDone;
}

GetHandler::~GetHandler(){};

}