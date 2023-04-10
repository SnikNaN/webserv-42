#pragma once

#include "IBodyHandler.hpp"

#include <fstream>

class UploadBodyHandler: public IBodyHandler
{
public:
    UploadBodyHandler(const std::string& path);
    void ProcessData(std::string &str);
    bool GetStatus();
    ~UploadBodyHandler();
private:
    bool m_isDone;
    std::ofstream m_dataStream;
};