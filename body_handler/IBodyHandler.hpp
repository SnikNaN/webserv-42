#pragma once

#include <string>

class IBodyHandler
{
public:
    virtual void ProcessData(std::string &str) = 0;
    virtual bool GetStatus() = 0;
    virtual ~IBodyHandler(){};
};