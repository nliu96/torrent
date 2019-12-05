#include "torrent/bencode.h"
#include "torrent/torrent.h"
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <openssl/sha.h>
#include <string>
#include <vector>

// namespace mini_bit {

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http;   // from <boost/beast/http.hpp>
namespace net = boost::asio;    // from <boost/asio.hpp>
using tcp = net::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

Bencode decode_torrent(std::vector<char>::iterator &begin,
                       std::vector<char>::iterator &end) {
  Bencode torrentDict = decode(begin, end);
  return torrentDict;
}

void load_file(std::string filename, std::vector<char> &buffer) {
  std::ifstream file(filename, std::ios::binary);
  std::streampos fileSize;
  file.unsetf(std::ios::skipws);
  file.seekg(0, std::ios::end);
  fileSize = file.tellg();
  file.seekg(0, std::ios::beg);
  buffer.reserve(fileSize);
  buffer.insert(buffer.begin(), std::istream_iterator<char>(file),
                std::istream_iterator<char>());
}

std::string url_encode(const std::string &value) {
  std::ostringstream encoded;
  encoded.fill('0');
  encoded << std::hex;

  for (std::string::const_iterator i = value.begin(); i != value.end(); i++) {
    std::string::value_type c = (*i);

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

// }; // namespace mini_bit

// Performs an HTTP GET and prints the response
int main(int argc, char **argv) {
  std::ifstream myfile;
  std::string torrentFilename = argv[1];
  std::vector<char> file_buffer;
  load_file(torrentFilename, file_buffer);

  std::vector<char>::iterator file_begin = file_buffer.begin();
  std::vector<char>::iterator file_end = file_buffer.end();

  Bencode file_bencode = decode_torrent(file_begin, file_end);
  mini_bit::Torrent torrent = mini_bit::Torrent(file_bencode);
  torrent.GetTracker();

  return EXIT_SUCCESS;
}
