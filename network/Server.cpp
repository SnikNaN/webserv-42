#include "Server.hpp"

#include <iostream>
#include <vector>
#include <arpa/inet.h>
#include <string>
#include <unistd.h>
#include <sys/select.h>
#include <algorithm>
#include "../parsers/ConfigParser.h"

namespace ft{

void Server::initialize(){

    m_configs = ConfigParser().get_config(m_configsPath);

    for (std::vector<CfgCtx>::iterator it = m_configs.begin(); it != m_configs.end(); it++)
    {
        struct sockaddr_in server_address;

        /* adress family */
        server_address.sin_family = AF_INET;

        /* string ip4 to binary and fill sockaddr_in */
        inet_pton(AF_INET, it->ip.c_str(), &(server_address.sin_addr));

        /* convert string port to binary and fill sockaddr_in */
        int port;
        std::stringstream(it->port) >> port;
        server_address.sin_port = htons(port);

        Shared_ptr<SocketHolder> socket_ptr;

        try
        {
            socket_ptr = Shared_ptr<SocketHolder>(new SocketHolder(AF_INET, SOCK_STREAM, 0, &m_configs));
        }
        catch(const std::exception &ex)
        {
            std::cerr << "Socket creation FAILED " << ex.what() << std::endl;
            throw std::exception();
        }

        try
        {
            bool alreadyHasPort = false;
            for(std::vector< ft::Shared_ptr<ft::SocketHolder> >::iterator it_socket = m_listenSockets.begin(); it_socket != m_listenSockets.end(); it_socket++)
            {
                if ((*it_socket)->getServerPort() == (*it).port)
                {
                    alreadyHasPort = true;
                }
            }
            if (alreadyHasPort)
                continue ;

            socket_ptr->bind(&server_address);
            socket_ptr->listen();
            socket_ptr->setNonBlocking();
            std::cout << "Binding to port " << (*it).port << std::endl;
        }
        catch(const std::exception &ex)
        {
            std::cerr << "Listener socket init FAILED " << ex.what() << std::endl;
            throw std::exception();
        }
  
        m_listenSockets.push_back(socket_ptr);
    }
}

void Server::initReadWriteSets(fd_set &read, fd_set &write)
{
    for (std::vector< Shared_ptr<SocketHolder> >::iterator it = m_listenSockets.begin(); it != m_listenSockets.end(); it++)
    {
        FD_SET((*it)->getFd(), &read);

        if (m_maxSelectFd <= (*it)->getFd())
        {
            m_maxSelectFd = (*it)->getFd() + 1;
        }
    }

    for (std::vector< Shared_ptr<SocketHolder> >::iterator it = m_rwSockets.begin(); it != m_rwSockets.end(); it++)
    {
        if ((*it)->getFd() > 0 && (*it)->getStatus() != Done)
        {
            FD_SET((*it)->getFd(), &write);
            FD_SET((*it)->getFd(), &read);
            if (m_maxSelectFd <= (*it)->getFd())
            {
                m_maxSelectFd = (*it)->getFd() + 1;
            }
        }
    }
}

Server::Server(const std::string& confPath): m_configsPath(confPath), m_maxSelectFd(0)
{

}

void Server::Run()
{
    initialize();

    fd_set readFd;
    fd_set writeFd;

	while(true)
    {
        FD_ZERO(&readFd);
        FD_ZERO(&writeFd);

        initReadWriteSets(readFd, writeFd);
        select(m_maxSelectFd, &readFd, &writeFd, NULL, NULL);

        /* accept all incoming connections and create read/write sockets */
        for (std::vector< Shared_ptr<SocketHolder> >::iterator it = m_listenSockets.begin(); it != m_listenSockets.end(); it++)
        {
            if ( FD_ISSET( (*it)->getFd(), &readFd) )
            {
                int fd = (*it)->accept_int();
                std::cout << "  socket #" << (*it)->getFd() << "accepting" << std::endl;
                if (fd != -1)
                {
                    Shared_ptr<SocketHolder> sh_h(new SocketHolder(fd, &m_configs));
                    std::cout << "Accepted " << sh_h->getFd() << "Status" << sh_h->getStatus() << std::endl;
                    sh_h->setNonBlocking();
					sh_h->SetMServerIp((*it)->getServerIp());
					sh_h->SetMServerPort((*it)->getServerPort());
                    m_rwSockets.push_back(sh_h);
                }
            }
            // std::cout << "Listener FD mark NO-READ " << (*it)->getFd() << std::endl;
        }

        /* READ REQ */
        for (std::vector< Shared_ptr<SocketHolder> > ::iterator it = m_rwSockets.begin(); it != m_rwSockets.end(); it++)
        {
            if ( FD_ISSET( (*it)->getFd(), &readFd) && (((*it)->getStatus() == ReadRequest || ((*it)->getStatus() == ReadBody))))
            {
                //std::cout << "socket #" << (*it)->getFd() << " SELECT READ" << std::endl;
                (*it)->ProcessRead();
            }
            if (( FD_ISSET( (*it)->getFd(), &writeFd) ) &&  ((*it)->getStatus() == WriteRequest))
            {
                (*it)->ProcessWrite();
            }
            if (((*it)->getStatus() == PrepareCgi) || ((*it)->getStatus() == ProcessCgi))
                (*it)->ProcessWrite();
            // std::cout << "RW SOCKET mark NO-READ " << (*it)->getFd() << std::endl;
        }

        for (std::vector< Shared_ptr<SocketHolder> >::iterator it = m_rwSockets.begin();
                    it != m_rwSockets.end();)
        {
            if ((*it)->getStatus() == Done)
            {
                std::cout << "socket #" << (*it)->getFd() << " deleted" << std::endl;
                it = m_rwSockets.erase(it);
            }
            else
                it++;
        }
    }
}
}