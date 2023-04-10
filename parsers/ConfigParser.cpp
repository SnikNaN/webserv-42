#include "ConfigParser.h"

#include <cstdlib>

namespace ft
{
    std::string ConfigParser::clean_comments(const std::string& str)
    {
        std::istringstream ist(str);
        std::string res;
        std::string tmp;

        while (std::getline(ist, tmp, '#'))
        {
            res += tmp;
            std::getline(ist, tmp);
        }
        
        return res;
    }

    std::string ConfigParser::get_server_block(std::string& str)
    {
        size_t start, end;

        start = str.find("server {");
        if (start == std::string::npos)
            return "";

        start = str.find("{", start);

        end = str.find("server {", start);

        end = str.find_last_of('}', end);

        std::string res = str.substr(start + 1, end - start - 1);

        str = str.substr(end, str.size() - end);

        return res;
    }

    std::string ConfigParser::get_location_block(std::string& str)
    {
        size_t  start, end, open_br, path_start;
        std::string location;

        open_br = str.find("{");
        end = str.find("}");
        if (open_br == std::string::npos)
            return "";
        start = str.rfind("location ", open_br);
        path_start = str.find_first_not_of(' ', start + 8);

        location = str.substr(path_start, end - path_start);
        str = str.substr(0, start) + str.substr(end + 1, str.size() - end - 1);
        
        open_br = location.find('{');
        location[open_br] = ' ';
        
        return location;
    }

    std::vector<CfgCtx> ConfigParser::get_config(const std::string& filename)
    {
        std::ifstream conf(filename.c_str(), std::ios::in);
        std::string tmp;

        std::getline(conf, tmp, '%');

        conf.close();

        tmp = clean_comments(tmp);

        std::string server;

        while (!(server = get_server_block(tmp)).empty())
        {
            CfgCtx v_server;
            v_server.ip = "0.0.0.0";
            v_server.port = "8080";

            std::string err_path("err_pages/");//debug
            v_server.error_pages.insert(std::make_pair(400, err_path + "400.html"));
            v_server.error_pages.insert(std::make_pair(403, err_path + "403.html"));
            v_server.error_pages.insert(std::make_pair(404, err_path + "404.html"));
            v_server.error_pages.insert(std::make_pair(405, err_path + "405.html"));
            v_server.error_pages.insert(std::make_pair(411, err_path + "411.html"));
            v_server.error_pages.insert(std::make_pair(413, err_path + "413.html"));
            v_server.error_pages.insert(std::make_pair(414, err_path + "414.html"));
            v_server.error_pages.insert(std::make_pair(500, err_path + "500.html"));
            v_server.error_pages.insert(std::make_pair(501, err_path + "501.html"));
            v_server.error_pages.insert(std::make_pair(502, err_path + "502.html"));
            v_server.error_pages.insert(std::make_pair(504, err_path + "504.html"));
            v_server.error_pages.insert(std::make_pair(505, err_path + "505.html"));

            std::string location;
            while (!(location = get_location_block(server)).empty())
            {
                Location    new_location;

                std::istringstream  iss_location(location);
                
                iss_location >> new_location.path;
                std::string l_params;
                while (std::getline(iss_location, l_params, ';'))
                {
                    std::istringstream  iss_loc_values(l_params);
                    std::string l_arg;

                    iss_loc_values >> l_arg;

                    if (l_arg == "root")
                    {
                        iss_loc_values >> new_location.root;
                    }
                    else if (l_arg == "return")
                    {
                        new_location.is_redirect = true;
                        //iss_loc_values >> new_location.return_code;
                        iss_loc_values >> new_location.redirect_uri;
                    }
                    else if (l_arg == "accepted_methods")
                    {
                        std::string method;
                        new_location.allow_get = false;
                        new_location.allow_post = false;
                        new_location.allow_del = false;
                        while (iss_loc_values >> method)
                        {
                            if (method == "GET")
                                new_location.allow_get = true;
                            else if (method == "POST")
                                new_location.allow_post = true;
                            else if (method == "DELETE")
                                new_location.allow_del = true;
                        }
                    }
                    else if (l_arg == "autoindex")
                    {
                        std::string allow;
                        iss_loc_values >> allow;
                        if (allow == "on")
                            new_location.autoindex = true;
                    }
                    else if (l_arg == "file_transfer")
                    {
						std::string allow;
						iss_loc_values >> allow;
						if (allow == "on")
                        	new_location.file_transfer = true;
                    }
                    else if (l_arg == "cgi")
                    {
                        new_location.allow_cgi = true;
                    }
                    else if (l_arg == "index")
					{
					  	iss_loc_values >> new_location.index;
					}
                    else if (l_arg == "client_max_body_size")
                    {
                        iss_loc_values >> new_location.client_max_body_size;
                    }
                }

                v_server.location_paths.insert(new_location.path);
                v_server.locations.insert(std::make_pair(new_location.path, new_location));
            }
            //get locations
            
            std::istringstream  iss_server_params(server);
            std::string param;
            
            while (std::getline(iss_server_params, param, ';'))
            {
                // if (key == "client_max_body_size")
                std::istringstream  iss_key_values(param);
                std::string key;
                
                iss_key_values >> key;

                if (key == "client_max_body_size")
                {
                    //std::string value;
                    //iss_key_values >> value;
                    iss_key_values >> v_server.client_max_body_size;
                    //v_server.client_max_body_size = atoi(value.c_str());
                }
                else if (key == "listen")
                {
                    std::string tmp_ip;
                    std::string tmp_port;

                    iss_key_values >> std::ws;
                    std::getline(iss_key_values, tmp_ip, ':');
                    if (tmp_ip.find('.') == std::string::npos)
                    {
                        v_server.port = tmp_ip;
                    }
                    else
                    {
                        v_server.ip = tmp_ip;
                        iss_key_values >> tmp_port;
                        if (!tmp_port.empty())
                            v_server.port = tmp_port;
                    }
                }
                else if (key == "server_name")
                {
                    std::string s_name;
                    while (iss_key_values >> s_name)
                        v_server.server_names.insert(s_name);
                }
                else if (key == "error_page")
                {
                    u_short err_code;
                    std::string err_page;
                    iss_key_values >> err_code >> err_page;
                    v_server.error_pages.find(err_code)->second = err_page;
                }
                else if (key == "root")
                {
                    iss_key_values >> v_server.root;
                }
            }

            for (std::map<std::string, Location>::iterator it = v_server.locations.begin();
                    it != v_server.locations.end(); ++it)
            {
                if ((*it).second.root.empty())
                    (*it).second.root = v_server.root;
                if ((*it).second.client_max_body_size == 0)
                    (*it).second.client_max_body_size = v_server.client_max_body_size;
            }
            m_config.push_back(v_server);
        }

        return m_config;
    }
}

// int main(int argc, char *argv[])
// {
//     ft::ConfigParser    cfg;
//     std::vector<ft::CfgCtx> vec = cfg.get_config(argv[1]);
//     std::vector<ft::CfgCtx>::iterator it1;

//     int i = 0;

//     for (it1 = vec.begin(); it1 != vec.end(); ++it1)
//     {
//         std::cout << "{SERVER [" << i << "]}:" << std::endl;
//         i++;
//         std::cout << (*it1).ip << ":" << (*it1).port << std::endl;
//         for (auto it2 = (*it1).locations.begin(); it2 != (*it1).locations.end(); ++it2)
//         {
//             std::cout << "location: " << (*it2).first << std::endl;
//             std::cout << (*it2).second.root << std::endl;
//         }
//     }

//     return 0;
// }