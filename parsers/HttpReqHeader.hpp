#pragma once

#include "../parsing/CfgCtx.hpp"

#include <iostream>
#include <string>
#include <sstream>
#include <map>

//==============================================================================//
//  ________  ______________    _____   ______  ___________________   ________  //
//  |scheme| |  hostname    |   |port|  |path|  |     query       |   |anchor|  //
//  |   ___| |______________|   |___|    |__|   |_________________|   |______|  //
//  |  |     |              |   |   |    |  |   |                 |   |      |  //
//  http :// www.hostname.com : port  /  path ? arg=val & arg2=val2 # fragment  //
//                                                                              //
//==============================================================================//

//==================================================//
//                                                  //
//      ANOTHER HOST WITH THE SAME SCHEME           //
//                                                  //
//      //<host> <path> [<query>] [<fragment>]      //
//                                                  //
//==================================================//

//==================================================//
//                                                  //
//      ABSOLUTE PATH WITHIN CURRENT HOST           //
//                                                  //
//      /<path> [<query>] [<fragment>]              //
//                                                  //
//==================================================//

//==================================================//
//                                                  //
//      RELATIVE PATH WITHIN CURRENT HOST           //
//                                                  //
//      <path> [<query>] [<fragment>]               //
//                                                  //
//==================================================//

//==================================================//
//                                                  //
//      ABSOLUTE PATH                               //
//                                                  //
//   <scheme> <host> [<path>] [<query>][<fragment>] //
//                                                  //
//==================================================//

namespace ft
{
    struct uri
    {
        std::string scheme;
        std::string hostname;
        std::string port;
        std::string path;
		std::string path_info;
        std::string query;
        std::string fragment;
    };

    struct request_headers
    {
        std::string method;

        std::string scheme;
        std::string version;
        std::string uri;

        std::string host;
        size_t      port;
		std::string port_str;
        std::string path;
		std::string path_info;

        std::string query;//std::map<std::string, std::string> query;
        std::string fragment;

        size_t  cont_length;
        std::string content_type;
		std::string boundary;
        
        bool    keep_alive;
        bool    is_chunked;
        bool    is_req_folder;
		bool	is_cgi;
    }; 

    class HttpReqHeader
    {
    private:
        struct uri  m_st_uri;
        std::string m_method;
        std::string m_uri;
        std::string m_version;

        std::string m_host;
        std::string m_port;
        
        std::string m_content_length;
        
        std::string m_content_type;
		std::string m_boundary;
        
        bool        m_keep_alive;
        bool        m_chunked;
        bool        m_req_folder;
        bool        m_rel_path;
		bool		m_cgi;
        
    public:
        HttpReqHeader() {};
        HttpReqHeader(const std::string& request);
        ~HttpReqHeader() {};

        int parse_uri(const std::string& uri);
        request_headers get_req_headers();
    };
}