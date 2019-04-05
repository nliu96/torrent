#include <fstream>
#include <iostream>
#include <vector>
#include <iomanip>
#include "torrent/bencode.h"
#include <openssl/sha.h>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

namespace mini_bit
{

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http;   // from <boost/beast/http.hpp>
namespace net = boost::asio;    // from <boost/asio.hpp>
using tcp = net::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

Bencode decode_torrent(std::vector<char>::iterator &begin,
                       std::vector<char>::iterator &end)
{
  Bencode torrentDict = decode(begin, end);
  std::string announce = torrentDict.getDict()["announce"].getString();
  return torrentDict;
}

void load_file(std::string filename, std::vector<char> &buffer)
{
  std::ifstream file(filename, std::ios::binary);
  std::streampos fileSize;
  file.unsetf(std::ios::skipws);
  file.seekg(0, std::ios::end);
  fileSize = file.tellg();
  file.seekg(0, std::ios::beg);
  buffer.reserve(fileSize);
  buffer.insert(buffer.begin(),
                std::istream_iterator<char>(file),
                std::istream_iterator<char>());
}

std::string url_encode(const std::string &value)
{
  std::ostringstream encoded;
  encoded.fill('0');
  encoded << std::hex;

  for (std::string::const_iterator i = value.begin(); i != value.end(); i++)
  {
    std::string::value_type c = (*i);

    // Keep alphanumeric and other accepted characters intact
    // if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
    //     encoded << c;
    //     continue;
    // }

    // Any other characters are percent-encoded
    // encoded << std::uppercase;
    encoded << '%' << std::setw(2) << int((unsigned char)c);
    encoded << std::nouppercase;
  }

  return encoded.str();
}

// Performs an HTTP GET and prints the response
int main(int argc, char **argv)
{
  try
  {
    std::ifstream myfile;
    std::string torrentFilename = argv[1];
    std::vector<char> file_buffer;
    load_file(torrentFilename, file_buffer);

    std::vector<char>::iterator file_begin = file_buffer.begin();
    std::vector<char>::iterator file_end = file_buffer.end();

    Bencode file_bencode = decode_torrent(file_begin, file_end);
    std::string infoString = encode(file_bencode.getDict()["info"]);

    unsigned char obuf[20];
    unsigned char val[infoString.length() + 1];
    std::memcpy(val, infoString.c_str(), infoString.length());
    SHA1(val, sizeof(val) - 1, obuf);
    std::string str(obuf, obuf + sizeof obuf / sizeof obuf[0]);
    std::cout << url_encode(str) << std::endl;

    auto const host = "torrent.ubuntu.com";
    auto const port = "6969";
    auto const target =
        "/announce?port=6881&downloaded=0&peer_id=%2D%41%5A%35%37%35%30%2D%"
        "54%70%6B%58%74%74%5A%4C%66%70%53%48&left=1923727360&info_hash=" +
        url_encode(str) + "&uploaded="
                          "0&compact=1&event=started";
    int version = argc == 3 && !std::strcmp("1.0", argv[2]) ? 10 : 11;

    // The io_context is required for all I/O
    net::io_context ioc;

    // These objects perform our I/O
    tcp::resolver resolver(ioc);
    beast::tcp_stream stream(ioc);

    // Look up the domain name
    auto const results = resolver.resolve(host, port);

    // Make the connection on the IP address we get from a lookup
    stream.connect(results);

    // Set up an HTTP GET request message
    http::request<http::string_body> req{http::verb::get, target, version};
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    // Send the HTTP request to the remote host
    http::write(stream, req);

    // This buffer is used for reading and must be persisted
    beast::flat_buffer buffer;

    // Declare a container to hold the response
    http::response<http::string_body> res;

    // Receive the HTTP response
    http::read(stream, buffer, res);

    // Write the message to standard out
    std::cout << res.body() << std::endl;
    std::string body = res.body();
    std::vector<char> v(body.begin(), body.end());
    std::vector<char>::iterator begin = v.begin();
    std::vector<char>::iterator end = v.end();

    Bencode ben = decode_torrent(begin, end);

    // Gracefully close the socket
    beast::error_code ec;
    stream.socket().shutdown(tcp::socket::shutdown_both, ec);

    // not_connected happens sometimes
    // so don't bother reporting it.
    //
    if (ec && ec != beast::errc::not_connected)
      throw beast::system_error{ec};

    // If we get here then the connection is closed gracefully
  }
  catch (std::exception const &e)
  {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

}; // namespace mini_bit
