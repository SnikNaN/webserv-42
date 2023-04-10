#ifndef WEB_SERVER_UTILS_UTILS_HPP_
#define WEB_SERVER_UTILS_UTILS_HPP_

#include <string>
#include <sstream>

enum  Errors
{
    NoError = 0,
    SocketError,
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

template <typename T>
std::string	to_string(T num)
{
	std::stringstream ss;
	ss << num;
	return ss.str();
}

//std::string	to_string(int num)
//{
//	std::stringstream ss;
//	ss << num;
//	return ss.str();
//}

#endif //WEB_SERVER_UTILS_UTILS_HPP_
