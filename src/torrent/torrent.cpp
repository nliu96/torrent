#include "torrent.h"

#include <iomanip>

#include <openssl/sha.h>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

namespace {

Bencode DecodeTorrent(std::vector<char> torrent_str) {
  std::vector<char>::iterator begin = torrent_str.begin();
  std::vector<char>::iterator end = torrent_str.end();
  Bencode torrentDict = decode(begin, end);

  return torrentDict;
}

std::string UrlEncode(const std::string &value) {
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

std::string GenerateInfoHash(const std::string &info) {
  unsigned char obuf[20];
  unsigned char val[info.length() + 1];
  std::memcpy(val, info.c_str(), info.length());
  SHA1(val, sizeof(val) - 1, obuf);
  std::string str(obuf, obuf + sizeof obuf / sizeof obuf[0]);
  std::cout << UrlEncode(str) << std::endl;
  return UrlEncode(str);
}

}

namespace mini_bit {

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http;   // from <boost/beast/http.hpp>
namespace net = boost::asio;    // from <boost/asio.hpp>
using tcp = net::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

Torrent::Torrent(Bencode torrent_info) {
  info_hash_ = GenerateInfoHash(encode(torrent_info.getDict()["info"]));
  announce_ = torrent_info.getDict()["announce"].getString();
  char prot[20], host[20], page[20];
  std::cout << "Announce " << announce_.c_str() << std::endl;
  sscanf(announce_.c_str(), "%[^:]://%[^:]:%d/%s", prot, host, &port_, page);
  std::cout << host << " " << port_ << " " << page << std::endl;
  host_ = std::string(host);
}

void Torrent::GetTracker() {
  try {
    // The io_context is required for all I/O
    net::io_context ioc;

    // These objects perform our I/O
    tcp::resolver resolver(ioc);
    beast::tcp_stream stream(ioc);

    // Look up the domain name
    auto const results = resolver.resolve(host_, std::to_string(port_));

    // Make the connection on the IP address we get from a lookup
    stream.connect(results);

    int version = 10;

    const auto target = "/announce?port=" + std::to_string(port_) + "&info_hash=" + info_hash_;
    std::cout << target<< std::endl;

    // Set up an HTTP GET request message
    http::request<http::string_body> req{http::verb::get, target, version};
    req.set(http::field::host, host_);
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

    Bencode ben = DecodeTorrent(v);

    // Gracefully close the socket
    beast::error_code ec;
    stream.socket().shutdown(tcp::socket::shutdown_both, ec);

    // not_connected happens sometimes
    // so don't bother reporting it.
    //
    if (ec && ec != beast::errc::not_connected)
      throw beast::system_error{ec};
  } catch (std::exception const &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}

} // namespace mini_bit
