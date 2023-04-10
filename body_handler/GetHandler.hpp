#pragma once

#include "IBodyHandler.hpp"
#include "../parsers/HttpReqHeader.hpp"

#include <fstream>


namespace ft
{

class GetHandler: public IBodyHandler
{
public:
    GetHandler(const struct request_headers& reqHeader);
    virtual void ProcessData(std::string &str);
    virtual bool GetData(std::string &str);
    virtual bool GetStatus();
    virtual ~GetHandler();
private:
    struct request_headers m_reqHeaders;
    std::ifstream m_stream;
    bool m_isDone;
};

}