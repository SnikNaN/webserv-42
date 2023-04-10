#include <iostream>
#include <cstdlib>

int main(int argc, char** argv)
{
	char* env = std::getenv("PATH_TRANSLATED");
	std::string path;

	if (env)
		path = env;

	size_t size = path.size();

	if (!path.empty())
		std::cout << "HTTP/1.1 200 OK\r\n"\
		"Content-Length: " << size + 1 << "\r\n\r\n"
		<< path << std::endl;
	else
		std::cout << "HTTP/1.1 200 OK\r\n"\
		"Content-Length: " << 6 << "\r\n\r\n"
				  << "Hello" << std::endl;

	return 0;
}