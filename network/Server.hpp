#pragma once

#include "../parsing/CfgCtx.hpp"
#include "SocketHolder.h"
#include "../utils/SharedPtr.hpp"

#include <sys/select.h>
#include <vector>

namespace ft
{

class Server
{
public:
    void Run();
    Server(const std::string& confPath);
private:
    void initReadWriteSets(fd_set &read, fd_set &write);
    std::vector<CfgCtx> m_configs;
    std::string m_configsPath;
    std::vector< Shared_ptr<SocketHolder> > m_listenSockets;
    std::vector< Shared_ptr<SocketHolder> > m_rwSockets;
    std::vector<SocketHolder> m_rwESockets;

    void initialize();
    ssize_t m_maxSelectFd;
};

} //namespace ft