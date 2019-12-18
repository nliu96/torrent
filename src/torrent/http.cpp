#include "http.h"

#include <iomanip>
#include <iostream>
#include <sstream>

#include <openssl/sha.h>

namespace {

size_t writeFunction(void *ptr, size_t size, size_t nmemb, std::string *data) {
  data->append((char *)ptr, size * nmemb);
  return size * nmemb;
}

} // namespace

namespace mini_bit {

std::string GetRequest(std::string host, int port, std::string target) {
  std::string get_request_url =
      "http://" + host + ":" + std::to_string(port) + target;

  auto curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_URL, get_request_url.c_str());
  curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
  curl_easy_setopt(curl, CURLOPT_USERPWD, "user:pass");
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.42.0");
  curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
  curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

  std::string response_string;
  std::string header_string;
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
  curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header_string);

  char *url;
  long response_code;
  double elapsed;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
  curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &elapsed);
  curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &url);

  curl_easy_perform(curl);
  curl_easy_cleanup(curl);
  curl = NULL;

  return response_string;
}

std::string UrlEncode(const std::vector<unsigned char> &value) {
  std::ostringstream encoded;
  encoded.fill('0');
  encoded << std::hex;

  for (char c : value) {
    // Keep alphanumeric and other accepted characters intact
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      encoded << c;
      continue;
    }

    // Any other characters are percent-encoded
    encoded << std::uppercase;
    encoded << '%' << std::setw(2) << int((unsigned char)c);
    encoded << std::nouppercase;
  }

  return encoded.str();
}

} // namespace mini_bit
