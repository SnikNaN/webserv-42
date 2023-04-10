#pragma once

#include <string>
#include <vector>
#include <set>
#include <map>
#include <sys/types.h>
namespace ft
{
    struct Location
    {
        std::string path;
        std::string root;
        std::string redirect_uri;
        size_t  client_max_body_size;
        bool    allow_get;
        bool    allow_post;
        bool    allow_del;
        bool    allow_cgi;
        bool    autoindex;
        bool    is_redirect;
        std::string index;
        bool    file_transfer;
        std::string upload_path;

		Location() : client_max_body_size(0),
					 allow_get(true),
					 allow_post(false),
					 allow_del(false),
					 allow_cgi(false),
					 autoindex(false),
					 is_redirect(false),
					 file_transfer(false) {}
    };

    // struct VirtualServer
    // {
    //     std::vector<Location>   locations;
    //     std::set<std::string> names;
    //     // добавить контейнер под пары host:port
    //     size_t  client_max_body_size;
    //     std::map<int, std::string> error_pages;//<код:путь> контейнер под error_pages
    // };

    struct CfgCtx
    {
        //CfgCtx(const std::string& ip, const std::string& port): ip(ip), port(port){};
        std::map<std::string, Location> locations;
        std::set<std::string> server_names;
        std::set<std::string> location_paths;
        size_t  client_max_body_size;
        std::map<u_short, std::string> error_pages;
        std::string ip;
        std::string port;
        std::string root;
    };
} // ft