#ifndef HTTP_H
#define HTTP_H

#include <string>
#include <vector>

#include <curl/curl.h>

namespace mini_bit {

std::string GetRequest(std::string host, int port, std::string target);

std::string UrlEncode(const std::vector<unsigned char> &value);

} // namespace mini_bit

#endif
