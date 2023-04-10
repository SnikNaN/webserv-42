#include <iostream>
#include "network/Server.hpp"

int main(int argc, char* argv[])
{
	//chdir("..");
	std::string config_file("webserv.conf");
	if (argc == 2)
    	config_file = argv[1];
	else if (argc > 2)
	{
		std::cerr << "Error: Invalid arguments" << std::endl;
		return -1;
	}

	if (access(config_file.c_str(), R_OK))
	{
		std::cerr << "Error: Can't open configuration file" << std::endl;
		return -1;
	}

	ft::Server(config_file).Run();

    return 0;
}