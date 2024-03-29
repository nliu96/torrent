#ifndef HTTP_H
#define HTTP_H

#include <string>

#include <curl/curl.h>

namespace mini_bit {

std::string GetRequest(std::string host, int port, std::string target);

}

#endif
