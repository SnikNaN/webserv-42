#include "SocketHolder.h"
#include <arpa/inet.h>
#include <bitset>
#include <fcntl.h>
#include <dirent.h>
#include "../utils/utils.hpp"//

#define CHUNKED_HEADER "HTTP/1.1 200 OK\r\n"\
"Transfer-Encoding: chunked\r\n"\
"Connection: keep-alive\r\n"                \
"\r\n"

#define DOWNLOAD_HEADER "HTTP/1.1 200 OK\r\n"\
"Transfer-Encoding: chunked\r\nContent-Disposition: attachment; filename="

#define UPLOAD_HEADER "HTTP/1.1 201 Created\r\nContent-Length: 4\r\n\r\nDone"

namespace ft
{

SocketHolder::SocketHolder(int desc, std::vector<CfgCtx>* ctxs) :
                                        m_file_descriptor(desc),
                                        m_hostSockAddrLen(0),
                                        m_error_resp(false),
										m_is_autoindex(false),
										m_is_file_transfer(false),
										m_configs(ctxs)
{
    m_procStatus = ReadRequest;
    memset(&m_hostSockAdd, 0, sizeof(m_hostSockAdd));
    m_sh_type = "RW";
}

int SocketHolder::getFd()
{
    return m_file_descriptor;
}

const std::string& SocketHolder::getServerIp() const
{
    return m_serverIp;
}

const std::string& SocketHolder::getServerPort() const
{
    return m_serverPort;
}

ProcessStatus SocketHolder::getStatus() const
{
    return m_procStatus;
}

void SocketHolder::setNonBlocking()
{
    int flags = fcntl(m_file_descriptor, F_GETFL);
    if (flags < 0)
    {
        std::cout << "fd: "<< m_file_descriptor << std::endl;
        perror("falgs error: ");
        throw std::runtime_error("Can`t get fd flags \n");
    }

    flags |= O_NONBLOCK;
    if (fcntl(m_file_descriptor, F_SETFL, O_NONBLOCK) <  0)
    {
        throw std::runtime_error("Can`t set nonblocking option \n");
    }
}

SocketHolder::SocketHolder(int domain, int type, int protocol, std::vector<CfgCtx>* ctxs) :
                                                                        m_procStatus(ReadRequest),
                                                                        m_error_resp(false),
																		m_is_autoindex(false),
																		m_is_file_transfer(false),
																		m_configs(ctxs)
{
    m_sh_type = "Listen";
    m_file_descriptor = ::socket(domain, type,  protocol);
    memset(&m_hostSockAdd, 0, sizeof(m_hostSockAdd));

    if (m_file_descriptor == -1)
    {
        throw std::runtime_error("Creating socket: FAILED\n");
    }

     int opt = 1;
     if ((setsockopt(m_file_descriptor, SOL_SOCKET,
                   SO_REUSEADDR, &opt,
                   sizeof(opt))) == -1)
                   {
                        shutdown(m_file_descriptor, SHUT_RDWR);
                        close(m_file_descriptor);
                        throw std::runtime_error("Socket options application FAILED\n");
                   }
}

void SocketHolder::bind(struct sockaddr_in *addr)
{
     if (::bind(m_file_descriptor, reinterpret_cast<const struct sockaddr *>(addr), sizeof(*addr)) == -1)
     {
        std::cout << m_file_descriptor << std::endl;
        perror("error:\n");
        throw std::runtime_error("Error while binding\n");
     }

    std::stringstream ss;
    ss << ntohs(addr->sin_port);

    m_serverPort = ss.str();
    m_serverIp = std::string(inet_ntoa((addr)->sin_addr));

    // std::cout << "binding" << m_serverIp << " " << m_serverPort << std::endl;
}

void SocketHolder::listen()
{
    if (::listen(m_file_descriptor, BACKLOG) == -1)
    {
        perror("error:");
        throw std::runtime_error("Error while listen");
    }
}

void SocketHolder::send(const std::string & str)
{
    int res;
    if ((res =::send(m_file_descriptor, str.c_str(), str.size(), 0)) == -1)
    {
        perror("error:");
        throw std::runtime_error("Error while listen");
    }
    std::cout << "send: " << res << std::endl;
}

std::string SocketHolder::read()
{
    int res;
    char buffer[1024];

    std::stringstream ss;

    

    res =::read(m_file_descriptor, &buffer, 1023);


    // while ((res =::read(m_file_descriptor, &buffer, 1023)) == 1023)
    // {
    //     buffer[1024] = '\0';
    //     ss << buffer;
    // }
    // if (res == -1)
    // {
    //     perror("error:");
    //     throw std::runtime_error("Error while reading socket");
    // }

    if (res < 0)
    {
        return std::string();
    }

    buffer[res] = '\0';

    return std::string(buffer);
}



// void SocketHolder::SetNextState()
// {
//     switch (m_procStatus)
//     {
//         case (ReadRequest):
//             std::cout << "now STEP: ReadBody" << std::endl;
//             m_procStatus = ReadBody;
//             break;
//         case (ReadBody):
//             std::cout << "now STEP: ReadDone" << std::endl;
//             m_procStatus = ReadDone;
//             break;
//         case (ReadDone):
//             std::cout << "now STEP: WriteRequest" << std::endl;
//             m_procStatus = WriteRequest;
//             break;
//         case (WriteRequest):
//             std::cout << "now STEP: WriteBody" << std::endl;
//             m_procStatus = WriteBody;
//             break;
//         case (WriteBody):
//             std::cout << "now STEP: WriteRequest" << std::endl;
//             m_procStatus = WriteRequest;
//             break;
//         case (Done):
//             std::cout << "now STEP: Done" << std::endl;
//             break;
//     }

// }

void SocketHolder::ProcessRead()
{
	if (m_procStatus == ReadRequest)
	{
		std::cout << "  socket #" << m_file_descriptor << " AccumulateRequest" << std::endl;
		try
		{
			AccumulateRequest();
		}
		catch (Errors err)
		{
			m_err = err;
			m_error_resp = true;
			m_procStatus = WriteRequest;
		}
		catch (const std::exception &ex)
		{
			std::cerr << ex.what() << std::endl;
			m_procStatus = Done;
		}
//		Errors res;
//        res = AccumulateRequest();
//        if (res != NoError && res != SocketError)
//        {
//            m_err = res;
//            m_error_resp = true;
//            m_procStatus = WriteRequest;
//        }
//        else if (res == SocketError)
//            return;
	}
	else if (m_procStatus == ReadBody)
	{
		std::cout << "  socket #" << m_file_descriptor << " HandleBody" << std::endl;
		HandleBody();
	}
//	else if (m_procStatus == PrepareCgi)
//	{
//		SetCgi();
//	}
//	else if (m_procStatus == ProcessCgi)
//	{
//
//	}

}

void SocketHolder::InitWriteHandler()
{
    if (m_writeHandler.get() == NULL)
    {
        if (m_error_resp)
        {
            if (m_vServer.error_pages.empty())
                SetVServer();
            m_writeHandler = Shared_ptr<IOutputHandler>
                    (new OutputChunkedHandler(m_file_descriptor,
                                              m_vServer.error_pages.find(m_err)->second,
                                              MakeErrorHeader(m_err)));
        }
        else if (m_vServer.locations.find(m_location)->second.is_redirect)
        {
            std::ostringstream ss;
            ss << "HTTP/1.1 302 Redirect\r\n";
            ss << "Location: ";
            ss << m_vServer.locations.find(m_location)->second.redirect_uri;
            ss << "\r\n\r\n";
            m_writeHandler = Shared_ptr<IOutputHandler>(new OutputRawHandler(m_file_descriptor,
                                                                             ss.str()));
        }
        else if (m_reqHeader->get_req_headers().is_cgi)
        {
            m_writeHandler = Shared_ptr<IOutputHandler>(new OutputRawHandler(m_file_descriptor,
                                                                             m_cgi_raw_out));
        }
        else if (m_is_autoindex)
        {
            std::ostringstream ss;
            ss << "HTTP/1.1 200 OK\r\nContent-length: ";
            ss << m_file.size();
            ss << "\r\nContent-type: text/html";
            ss << "\r\n\r\n";
            ss << m_file;
            m_writeHandler = Shared_ptr<IOutputHandler>(new OutputRawHandler(m_file_descriptor,
                                                                             ss.str()));
        }
        else if (m_reqHeader->get_req_headers().method == "DELETE")
		{
			m_writeHandler = Shared_ptr<IOutputHandler>(new OutputRawHandler(m_file_descriptor,
																			 m_delete_resp));
		}
		else if (m_is_file_transfer)
		{
			if (m_reqHeader->get_req_headers().method == "GET")
			{
				std::string dwnld_header = DOWNLOAD_HEADER;
				std::string filename = m_file.substr(m_vServer.locations.find(m_location)->second.root.size());
				if (filename[0] == '/')
					filename.erase(0, 1);
				dwnld_header += "\"" + filename + "\"";
				dwnld_header.append("\r\n\r\n");
				try
				{
					m_writeHandler = Shared_ptr<IOutputHandler>
						(new OutputChunkedHandler(m_file_descriptor,
												  m_file,
												  dwnld_header));
				}
				catch (Errors err)
				{
					m_writeHandler = Shared_ptr<IOutputHandler>
						(new OutputChunkedHandler(m_file_descriptor,
												  m_vServer.error_pages.find(err)->second,
												  MakeErrorHeader(err)));
				}
			}
			else
			{
				m_writeHandler = Shared_ptr<IOutputHandler>(new OutputRawHandler(m_file_descriptor,
																				 UPLOAD_HEADER));
			}
		}
		else
        {
            try
            {
                m_writeHandler = Shared_ptr<IOutputHandler>
                    (new OutputChunkedHandler(m_file_descriptor,
                                              m_file,
                                              CHUNKED_HEADER));
            }
            catch (Errors err)
            {
                m_writeHandler = Shared_ptr<IOutputHandler>
                        (new OutputChunkedHandler(m_file_descriptor,
                                                  m_vServer.error_pages.find(err)->second,
                                                  MakeErrorHeader(err)));
            }
        }
    }
}

void SocketHolder::ProcessWrite()
{

    /* to do make desition depending on state*/
    if (m_procStatus == WriteRequest)
    {
        InitWriteHandler();
        try
        {
            m_writeHandler->ProcessOutput();
        }
        catch (const std::exception &ex)
        {
            std::cerr << ex.what() << std::endl;
            m_procStatus = Done;
        }
        catch (...)
        {
            std::cerr << "exception\n";
        }
        if (m_writeHandler->IsDone())
        {
            std::cout << "  socket #" << m_file_descriptor << "ProcessWrite" << "status = Done" << std::endl;
            m_procStatus = Done;
        }
    }
	else if (m_procStatus == PrepareCgi)
	{
		SetCgi();
	}
	else if (m_procStatus == ProcessCgi)
	{
		HandleCgi();
	}
}

int SocketHolder::accept_int()
{
    return ::accept(m_file_descriptor, &m_hostSockAdd, &m_hostSockAddrLen);
}

Shared_ptr<SocketHolder> SocketHolder::accept()
{
    m_res = ::accept(m_file_descriptor, &m_hostSockAdd, &m_hostSockAddrLen);

    // std::cout << res << std::endl;
    // SocketHolder sock(res);
    Shared_ptr<SocketHolder> sock(new SocketHolder(m_res, m_configs));

    // std::cout << "Status::: " << std::endl;

    /* todo debug */
    if (sock->getFd() != -1)
    {
//		std::stringstream ss;
//		ss << ntohs(reinterpret_cast<sockaddr_in*>(&m_hostSockAdd)->sin_port);
//		ss >> m_serverPort;
//		m_serverIp = std::string(inet_ntoa(reinterpret_cast<sockaddr_in*>(&m_hostSockAdd)->sin_addr));
		// m_ip_port = std::string(inet_ntoa(reinterpret_cast<sockaddr_in*>(&m_hostSockAdd)->sin_addr))
		// 			+ ":";
		// m_ip_port.append(ss.str());
        std::cout << "NEW SOCKET ACCEPTED" << std::endl;
        std::cout << "  Host Port:" << m_serverIp << " : " << m_serverPort << std::endl;
        std::cout << "  FD:" << sock->getFd() << std::endl << std::endl;
	}
    else
    {
    // std::cout << reinterpret_cast<sockaddr_in*>(&m_hostSockAdd)-> << std::endl;
    //    throw std::runtime_error("Error accepting socket");
        sock->m_procStatus = Done;
    }

    return sock;
}

SocketHolder::~SocketHolder()
{

    std::cout << std::endl << "<<<<<<<socket destroyed " << m_file_descriptor << std::endl;
    // std::cout << "free " << m_file_descriptor << " objc " << *m_obj_counter <<  std::endl;
    if (m_file_descriptor != -1)
    {
        ::shutdown(m_file_descriptor, SHUT_RDWR);
        ::close(m_file_descriptor);
    }
    else
        std::cout << "-1";
}

void SocketHolder::AccumulateRequest()
{
    char buffer[BUFF_SIZE];
   
    ssize_t res = recv(m_file_descriptor, buffer, BUFF_SIZE, 0);

    if (res <= 0)
    {
        // std::cout << m_file_descriptor << std::endl;
        perror("FAILED");
        throw std::runtime_error("AccumulateRequest FAILED");
    }

	m_req_string.append(buffer, res);

    std::string::size_type found = m_req_string.find("\r\n\r\n");

//    if (m_req_string.find("\r\n") != std::string::npos)
//        if (m_req_string.find("HTTP/1.1") == std::string::npos)
//            throw BadRequest;

    /* if req header done reading */
    if (found != std::string::npos)
    {
		m_remainAfterRequest = m_req_string.substr(found + 4, m_req_string.size() - (found + 4));

		m_req_string = m_req_string.substr(0, found);
        /*todo testing only. should be READ BODY*/
        m_reqHeader = Shared_ptr<HttpReqHeader>(new HttpReqHeader(m_req_string));

        request_headers hdrs = m_reqHeader->get_req_headers();

        SetVServer();

        if (hdrs.method != "GET" && hdrs.method != "POST" && hdrs.method != "DELETE")
            throw NotImplemented;
        if (!hdrs.scheme.empty() && hdrs.scheme != "http")
            throw BadRequest;
//        if (hdrs.version != "HTTP/1.1")
//            throw HttpVersionNS;
        if (hdrs.uri.size() > 2000)
            throw ReqUriTooLong;

        SetLocation();
        if (m_location.empty())
            throw NotFound;

        if (m_vServer.locations.find(m_location)->second.is_redirect)
        {
            m_procStatus = WriteRequest;
            return;
        }

		m_is_file_transfer = m_vServer.locations.find(m_location)->second.file_transfer;

		if (!m_is_autoindex && access(m_file.c_str(), F_OK) != 0)
			throw NotFound;

		if (hdrs.method == "POST")
        {
            if (m_vServer.locations.find(m_location)->second.allow_post)
            {
                size_t max_body_size = m_vServer.locations.find(m_location)->second.client_max_body_size;
                if (max_body_size)
                {
                    if (hdrs.is_chunked)
                        throw LengthReq;
                    else if (hdrs.cont_length > max_body_size)
                        throw ReqEntTooLarge;
                }

                m_procStatus = ReadBody;
                InitBodyHandler();
                if (m_bodyHandler->IsDone())
                {
                    m_body = dynamic_cast<InputLengthHandler *>(m_bodyHandler.get())->GetRes();
					if (!m_reqHeader->get_req_headers().is_cgi && !m_reqHeader->get_req_headers().boundary.empty())
					{
						std::string filename;

						size_t begin, end;
						//std::string s = dynamic_cast<InputLengthHandler *>(m_bodyHandler.get())->GetRes();

						begin = m_body.find("; filename=\"");

						end = m_body.find("\r\n", begin);

						filename = m_body.substr(begin + sizeof("; filename="), end - begin - sizeof("; filename=") - 1);

						filename = m_vServer.locations.find(m_location)->second.root + "/" + filename;

						begin = m_body.find("\r\n\r\n", end) + 4;
						end = m_body.find("--" + m_reqHeader->get_req_headers().boundary, begin);

						std::fstream file(filename.c_str(), std::ios::out);
						for (size_t i = begin; i < end; ++i) {
							file.put(m_body[i]);
						}
						file.close();
					}
                    if (hdrs.is_cgi)
					{
						if (access(m_file.c_str(), X_OK) != 0)
							throw Forbidden;
                        m_procStatus = PrepareCgi;
					}
                    else
                        m_procStatus = WriteRequest;
                }
            }
            else
                throw MethodNA;
        }
		else if (hdrs.method == "GET")
        {
            if (m_vServer.locations.find(m_location)->second.allow_get)
            {
                if (hdrs.is_cgi)
				{
					if (access(m_file.c_str(), X_OK) != 0)
						throw Forbidden;
					m_procStatus = PrepareCgi;
				}
                else
                    m_procStatus = WriteRequest;
            }
            else
                throw MethodNA;
        }
        else if (hdrs.method == "DELETE")
		{
			if (m_vServer.locations.find(m_location)->second.allow_del)
			{
				if (!m_is_autoindex)
				{
					if (remove(m_file.c_str()) < 0)
						throw IntServerErr;
					m_delete_resp = "HTTP/1.1 200 OK\nContent-Length: 7\r\n\r\nDeleted";
					m_procStatus = WriteRequest;
				}
				else
					throw Forbidden;
			}
			else
				throw MethodNA;
		}
    }
}

 void SocketHolder::InitBodyHandler()
 {
    //   std::cout << "<<<<<<<<INIT BODYHANDLER" << std::endl;
     if (m_bodyHandler.get() == NULL)
     {
        if(m_reqHeader->get_req_headers().cont_length > 0)
        {
		 m_bodyHandler = Shared_ptr<IInputHandler>(new InputLengthHandler
			 (m_file_descriptor, m_reqHeader->get_req_headers().cont_length, m_remainAfterRequest));
         std::cout << "  socket #" << m_file_descriptor << "<<<<<<<<INIT BODYHANDLER InputLengthHandler" << std::endl;
        }
        else if(m_reqHeader->get_req_headers().cont_length == 0)
        {
            if (m_reqHeader->get_req_headers().is_chunked)
            {
                std::cout << "  socket #" << m_file_descriptor << "<<<<<<<<INIT BODYHANDLER InputChunkedHandler" << std::endl;
                m_bodyHandler = Shared_ptr<IInputHandler>(new InputChunkedHandler
			        (m_file_descriptor));
            }
            else
            {
                m_procStatus = Done;
                // std::cout << "<<<<<<SET STATUS DONE" << std::endl;
                //m_bodyHandler = Shared_ptr<IInputHandler>(new InputLengthHandler
                        //(m_file_descriptor, 0, m_remainAfterRequest));
            }
        }
     }
 }

 void SocketHolder::HandleBody()
 {
     InitBodyHandler();

     std::cout << "HANDLE BODY" << std::endl;
     try
     {
        m_bodyHandler->ProcessInput();
     }
     catch (const std::exception &ex)
     {
         std::cerr << ex.what() << std::endl;
         m_procStatus = Done;
     }
     if (m_bodyHandler->IsDone())
     {
		 m_body = dynamic_cast<InputLengthHandler *>(m_bodyHandler.get())->GetRes();
		 if (!m_reqHeader->get_req_headers().is_cgi && !m_reqHeader->get_req_headers().boundary.empty())
		 {
			 std::string filename;

			 size_t begin, end;
			 //std::string s = dynamic_cast<InputLengthHandler *>(m_bodyHandler.get())->GetRes();

			 begin = m_body.find("; filename=\"");

			 end = m_body.find("\r\n", begin);

			 filename = m_body.substr(begin + sizeof("; filename="), end - begin - sizeof("; filename=") - 1);

			 filename = m_vServer.locations.find(m_location)->second.root + "/" + filename;

			 begin = m_body.find("\r\n\r\n", end) + 4;
			 end = m_body.find("--" + m_reqHeader->get_req_headers().boundary, begin);

			 std::fstream file(filename.c_str(), std::ios::out);
			 for (size_t i = begin; i < end; ++i) {
				 file.put(m_body[i]);
			 }
			 file.close();
		 }
		 if (m_reqHeader->get_req_headers().is_cgi)
			 m_procStatus = PrepareCgi;
		 else
			 m_procStatus = WriteRequest;
     }
 }

void SocketHolder::SetVServer()
{
	std::vector<CfgCtx>::const_iterator cit;
	std::vector<CfgCtx>::iterator it;
	std::vector<CfgCtx> match;
	std::string		host;

	if (m_reqHeader.get() != NULL)
	  host = m_reqHeader->get_req_headers().host;

	std::string	server_ip_port;
	std::string m_ip_port = m_serverIp + ":" + m_serverPort;

	for (cit = m_configs->begin(); cit != m_configs->end(); ++cit)
	{
		server_ip_port = cit->ip + ":" + cit->port;

		if ((server_ip_port == m_ip_port) || (cit->ip == "0.0.0.0" && (cit->port == m_serverPort)))
			match.push_back(*cit);
	}

	for (it = match.begin(); it != match.end(); ++it)
	{
		if (it->server_names.find(host) != it->server_names.end())
		{
			m_vServer = *it;
			return;
		}
	}

	m_vServer = *match.begin();
}

void SocketHolder::SetLocation()
{
	m_file = m_reqHeader->get_req_headers().path;
	bool isdir = m_reqHeader->get_req_headers().is_req_folder;
	bool isCgi = m_reqHeader->get_req_headers().is_cgi;

	std::set<std::string>::reverse_iterator it;

	for (it = m_vServer.location_paths.rbegin(); it != m_vServer.location_paths.rend(); ++it)
	{
		if ((m_file + "/").find(*it) != std::string::npos)
		{
			m_location = *it;
			break;
		}
	}

	if (m_location.empty())
        return;
	if (m_file.size() > m_location.size())
		m_file = m_file.substr(m_location.size());
	else
		m_file.clear();

	if (!isdir)
	{
		if (IsDir((m_vServer.locations.find(m_location)->second.root + "/" + m_file).c_str()))
			throw NotFound;
	}

	if (isdir && !isCgi)
	{
        if (m_vServer.locations.find(m_location)->second.autoindex)
        {
			m_is_autoindex = true;
            m_file = MakeAutoindex();
        }
        else
        {
			if (m_vServer.locations.find(m_location)->second.index.empty())
				throw NotFound;
			if (access((m_vServer.locations.find(m_location)->second.root + "/" + m_file).c_str(), F_OK))
				throw NotFound;
		    m_file = m_vServer.locations.find(m_location)->second.root
			    + "/" + m_vServer.locations.find(m_location)->second.index;
        }
	}
	else
		m_file = m_vServer.locations.find(m_location)->second.root + "/" + m_file;
}

void SocketHolder::SetMServerIp(const std::string &m_server_ip)
{
	m_serverIp = m_server_ip;
}

void SocketHolder::SetMServerPort(const std::string &m_server_port)
{
	m_serverPort = m_server_port;
}
void SocketHolder::SetCgi()
{
	request_headers hdrs = m_reqHeader->get_req_headers();
	m_envp[0] = strdup("GATEWAY_INTERFACE=CGI/1.1");
	m_envp[1] = strdup(("PATH_INFO=" + hdrs.path_info).c_str());
	m_envp[2] = strdup(("QUERY_STRING=" + hdrs.query).c_str());
	m_envp[3] = strdup(("REQUEST_METHOD=" + hdrs.method).c_str());
	m_envp[4] = strdup(("SCRIPT_NAME=" + hdrs.path).c_str());
	m_envp[5] = strdup(("SERVER_NAME=" + hdrs.host).c_str());
	m_envp[6] = strdup(("SERVER_PORT=" + hdrs.port_str).c_str());
	m_envp[7] = strdup("SERVER_PROTOCOL=HTTP/1.1");
	char buf[1024];
	getcwd(buf, 1024);
	std::string wd = buf;
	m_envp[8] = strdup(("PATH_TRANSLATED="
						+ wd + "/" + m_vServer.locations.find(m_location)->second.root
						+ hdrs.path_info).c_str());
	m_envp[9] = strdup(("SCRIPT_FILENAME=" + m_file).c_str());
	m_envp[10] = strdup(("CONTENT_TYPE=" + hdrs.content_type).c_str());
	if (hdrs.cont_length != 0)
		m_envp[11] = strdup(("CONTENT_LENGTH=" + to_string(hdrs.cont_length)).c_str());
	else if (hdrs.method == "POST")
	{
		size_t cont_length = m_body.size();
		if (cont_length > 0)
			m_envp[11] = strdup(("CONTENT_LENGTH=" + to_string(cont_length)).c_str());
	}
	else
		m_envp[11] = strdup("CONTENT_LENGTH=");
	m_envp[12] = NULL;

	m_argv[0] = strdup((m_file).c_str());
	m_argv[1] = NULL;

    m_procStatus = ProcessCgi;

	if (hdrs.method == "POST")
    {
		m_cgiHandler = Shared_ptr<IInputHandler>(new InputCgiPostHandler);
        try
        {
            dynamic_cast<InputCgiPostHandler *>
            (m_cgiHandler.get())->Init(m_envp, m_argv, m_body, m_file_descriptor);
        }
        catch (Errors err)
        {
            m_err = err;
            m_error_resp = true;
            m_procStatus = WriteRequest;
        }
    }
	else if (hdrs.method == "GET")
    {
        m_cgiHandler = Shared_ptr<IInputHandler>(new InputCgiGetHandler);
        try
        {
            dynamic_cast<InputCgiGetHandler *>
            (m_cgiHandler.get())->Init(m_envp, m_argv, m_file_descriptor);
        }
        catch (Errors err)
        {
            m_err = err;
            m_error_resp = true;
            m_procStatus = WriteRequest;
        }
    }
    for (size_t i = 0; m_envp[i] != NULL; ++i)
    {
        delete (m_envp[i]);
    }
    delete m_argv[0];
}
void SocketHolder::HandleCgi()
{
    try
    {
        m_cgiHandler->ProcessInput();
    }
    catch (Errors err)
    {
        m_err = err;
        m_error_resp = true;
        m_procStatus = WriteRequest;
    }
	if (m_cgiHandler->IsDone())
	{
		if (m_reqHeader->get_req_headers().method == "GET")
//            m_cgi_raw_out = "HTTP/1.1 200 OK\r\nContent-Length: 6\r\n\r\nHello\n";
			m_cgi_raw_out = dynamic_cast<InputCgiGetHandler *>(m_cgiHandler.get())->GetRes();
		else
		{
//            m_cgi_raw_out = "HTTP/1.1 200 OK\r\nContent-Length: 6\r\n\r\nPostQ\n";
            m_cgi_raw_out = dynamic_cast<InputCgiPostHandler *>(m_cgiHandler.get())->GetRes();
			std::stringstream hdr;
			hdr << "HTTP/1.1 200 OK\r\nContent-length: ";
			hdr << m_cgi_raw_out.size();
			hdr << "\r\n\r\n";
			m_cgi_raw_out = hdr.str() + m_cgi_raw_out;
		}
		m_procStatus = WriteRequest;
	}
}

    std::string SocketHolder::MakeAutoindex()
    {
        std::string autoIndex;
        char    buf[BUFFER];
        
        autoIndex += "<!DOCTYPE html>\n";
        autoIndex += "<html>\n";
        autoIndex += "<head>\n";
        autoIndex += "<meta http-equiv=\"Content\" content=\"text/html; charset=UTF-8\">\n";
        autoIndex += "</head>\n";
        autoIndex += "<body>\n";
        autoIndex += "<table>\n";
        autoIndex += "<tbody id=\"tbody\">\n";

        DIR *dir;
        std::string slash;
        struct dirent *ent;
        bzero(buf, BUFFER);
        std::string dirbuf(getcwd(buf, BUFFER));
        dirbuf.append("/");
        dirbuf.append(m_vServer.locations.find(m_location)->second.root);
		if (dirbuf[dirbuf.size() - 1] != '/')
			dirbuf.append("/");
		dirbuf.append(m_file);
		if (dirbuf[dirbuf.size() - 1] != '/')
			dirbuf.append("/");
		if (access(dirbuf.c_str(), F_OK) != 0)
			throw NotFound;
        if (IsDir(dirbuf.c_str()))
		{
			if (access(dirbuf.c_str(), R_OK) != 0)
				throw Forbidden;
            if ((dir = opendir(dirbuf.c_str())) != NULL)
			{
                while((ent = readdir(dir)) != NULL)
				{
					slash = "";
					std::string tmp(ent->d_name);
					if (IsDir((dirbuf + tmp).c_str()))
						slash = "/";
					if (tmp != ".")
					{
                        autoIndex += "<tr><td><form method=\"GET\"action=\"\"> <a href=\"";
						autoIndex += tmp + slash;
						autoIndex += "\">" + tmp + "</a></form></td>\n";
					}
                }
            }
            else
            {
				throw NotFound;
                //404;
            }
            closedir(dir);
        }
        else
		{
            autoIndex.clear();
            return autoIndex;
        }
        autoIndex += "</tbody>\n";
        autoIndex += "</table>\n";
        autoIndex += "</body>\n";
        autoIndex += "</html>";
        return autoIndex;
    }

    bool SocketHolder::IsDir(const char *path) {
        struct stat s = {};
        if (lstat(path, &s) == -1) {
            return false;
        }
        return S_ISDIR(s.st_mode);
    }

    std::string SocketHolder::MakeErrorHeader(u_short code)
    {
        std::string error("HTTP/1.1 ");
        switch (code)
        {
            case 400:
                error.append("400 Bad Request\r\n");
                break;
            case 403:
                error.append("403 Forbidden\r\n");
                break;
            case 404:
                error.append("404 Not Found\r\n");
                break;
            case 405:
                error.append("405 Method Not Allowed\r\n");
                break;
            case 411:
                error.append("411 Length Required\r\n");
                break;
            case 413:
                error.append("413 Request Entity Too Large\r\n");
                break;
            case 414:
                error.append("414 Request-URI Too Long\r\n");
                break;
            case 500:
                error.append("500 Internal Server Error\r\n");
                break;
            case 501:
                error.append("501 Not Implemented\r\n");
                break;
            case 502:
                error.append("502 Bad Gateway\r\n");
                break;
            case 504:
                error.append("504 Gateway Timeout\r\n");
                break;
            case 505:
                error.append("505 HTTP Version Not Supported\r\n");
                break;
            default:
                break;
        }

        error.append("Content-type: text/html\r\n"\
                        "Transfer-Encoding: chunked\r\n"\
                        "Connection: close\r\n\r\n");
        return error;
    }

} //namespace ft
