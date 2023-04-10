#pragma once

#include "../utils/SharedPtr.hpp"
#include "../body_handler/IBodyHandler.hpp"
#include "../body_handler/UploadBodyHandler.hpp"
#include "../process/Handler.hpp"
#include "../parsers/HttpReqHeader.hpp"
#include "../parsing/CfgCtx.hpp"

#include <netinet/in.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <sys/stat.h>

#include "../utils/utils.hpp"

#define PORT 8080
#define BACKLOG 30
#define BUFFER 2048


namespace ft{

enum ProcessStatus
{
    ReadRequest,
    ReadBody,
	PrepareCgi,
	ProcessCgi,
    WriteRequest,
    Done
};

class SocketHolder
{
public:
    SocketHolder(int domain, int type, int protocol, std::vector<CfgCtx>* ctxs);
    SocketHolder(int desc, std::vector<CfgCtx>* ctxs);

    ~SocketHolder();
    void bind(struct sockaddr_in *addr);
    void listen();
    Shared_ptr<SocketHolder> accept();

    int accept_int();

    void send(const std::string&);
    std::string read();
    ProcessStatus getStatus() const;
    void setNonBlocking();
    int getFd();
    void ProcessRead();
    void ProcessWrite();
    const std::string& getServerIp() const;
    const std::string& getServerPort() const;

private:
    SocketHolder();
    // void SetNextState();
    void AccumulateRequest();
    void HandleBody(void);
	void SetCgi();
	void HandleCgi();
    std::string MakeAutoindex();
    static bool IsDir(const char* path);
    static std::string MakeErrorHeader(u_short code);
    // bool m_req_done = false;

    /* whole request as string */
    std::string m_req_string;
    std::string m_remainAfterRequest;

    /* parsed req header*/
    Shared_ptr<HttpReqHeader> m_reqHeader;

    void InitBodyHandler();
    void InitWriteHandler();
	void SetVServer();
	void SetLocation();

     Shared_ptr<IInputHandler> m_bodyHandler;
    Shared_ptr<IOutputHandler> m_writeHandler;
	Shared_ptr<IInputHandler> m_cgiHandler;


    int m_file_descriptor;

    /* host info */
    sockaddr m_hostSockAdd;
    uint32_t m_hostSockAddrLen;

	std::string m_serverIp;
public:
	void SetMServerIp(const std::string &m_server_ip);
	void SetMServerPort(const std::string &m_server_port);
private:
	std::string m_serverPort;

    ProcessStatus m_procStatus;

	std::string m_curPath;
	std::string m_file;

	char* m_envp[13];
	char* m_argv[2];

	CfgCtx m_vServer;
	std::string m_location;
	std::string m_body;
	std::string m_cgi_raw_out;

	std::string m_delete_resp;

    std::string m_sh_type;

    int m_res;
    Errors m_err;
    bool   m_error_resp;

	bool	m_is_autoindex;
	bool	m_is_file_transfer;
    /* configs */
    std::vector<CfgCtx>* m_configs;
};

} // namespace ft