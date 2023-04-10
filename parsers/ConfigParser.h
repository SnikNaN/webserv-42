#pragma once

#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <sstream>
#include <vector>

#include "../parsing/CfgCtx.hpp"

/*
// default host: "localhost"
// default port: "80"
server {
    [listen: [host]:[port] [host]:[port];]
    [server_name: name1 name2;]

    root /path; # works for all locations without own root directive

    client_max_body_size (1m default, 0 unlimit);

    return [];

    error_page код ... [=[ответ]] uri;

    location {
        allowed_methods GET POST DELETE;
        autoindex on/off(default off);
        index file1 file2 file3; // предлагаю упростить до одного файла

        return [error_code] or [redirect_code redirect_uri];
    }
}

If a request ends with a slash, NGINX treats it as a request for a directory
and tries to find an index file in the directory. The index directive defines
the index file’s name (the default value is index.html). To continue with the
example, if the request URI is /images/some/path/, NGINX delivers the file
/www/data/images/some/path/index.html if it exists.
If it does not, NGINX returns HTTP code 404 (Not Found) by default.
To configure NGINX to return an automatically generated directory listing instead,
include the on parameter to the autoindex directive:

*/
namespace ft
{
    enum  Errors
    {
        BadRequest = 400,
        Forbidden = 403,
        NotFound = 404,
        MethodNA = 405,
        LengthReq = 411,
        ReqEntTooLarge = 413,
        ReqUriTooLong = 414,
        IntServerErr = 500,
        NotImplemented = 501,
        BadGateway = 502,
        GatewayTimeout = 504,
        HttpVersionNS = 505
    };

    class ConfigParser
    {
    private:
        std::vector<CfgCtx> m_config;
        std::string get_server_block(std::string& str);
        std::string clean_comments(const std::string& str);
        std::string get_location_block(std::string& str);
    public:
        ConfigParser() {};
        ~ConfigParser() {};
        std::vector<CfgCtx> get_config(const std::string& filename);
    };
}