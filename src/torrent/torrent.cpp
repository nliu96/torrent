#include "torrent.h"

#include <iomanip>

#include <openssl/sha.h>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

namespace beast = boost::beast;
namespace http = beast::http;  
namespace net = boost::asio;   
using tcp = net::ip::tcp;      

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
  return UrlEncode(str);
}

std::vector<std::pair<net::ip::address, int>> ParsePeers(Bencode peers) {
  std::vector<std::pair<net::ip::address, int>> peers_list;
  std::string peers_str = peers.getString();
  std::vector<unsigned char> peers_bytes(peers_str.begin(), peers_str.end());
  for (size_t i = 0; i < peers_bytes.size(); i += 6) {
    std::string addr_byte_0 = std::to_string((int) peers_bytes[i + 0]);
    std::string addr_byte_1 = std::to_string((int) peers_bytes[i + 1]);
    std::string addr_byte_2 = std::to_string((int) peers_bytes[i + 2]);
    std::string addr_byte_3 = std::to_string((int) peers_bytes[i + 3]);
    std::string peer_address =
        addr_byte_0 + "." + addr_byte_1 + "." + addr_byte_2 + "." + addr_byte_3;
    net::ip::address ip_address = net::ip::address::from_string(peer_address);
    int port_byte_0 = (int) peers_bytes[i+4];
    int port_byte_1 = (int) peers_bytes[i+5];
    int port = (port_byte_0 << 8) + port_byte_1;
    peers_list.push_back( std::make_pair(ip_address, port));
  }
  return peers_list;
}

}  // namespace

namespace mini_bit {

Torrent::Torrent(Bencode torrent_info) {
  info_hash_ = GenerateInfoHash(encode(torrent_info.getDict()["info"]));
  announce_ = torrent_info.getDict()["announce"].getString();
  char prot[20], host[20], page[20];
  sscanf(announce_.c_str(), "%[^:]://%[^:]:%d/%s", prot, host, &port_, page);
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

    std::string target =
        "/announce?info_hash=" + info_hash_ +
        "&peer_id=ABCDEFGHIJKLMNOPQRST&compact=1";

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
    std::vector<std::pair<net::ip::address, int>> peers = ParsePeers(ben.getDict()["peers"]);

    // Gracefully close the socket
    beast::error_code ec;
    stream.socket().shutdown(tcp::socket::shutdown_both, ec);

    // not_connected happens sometimes
    // so don't bother reporting it.
    //
    if (ec && ec != beast::errc::not_connected) throw beast::system_error{ec};
  } catch (std::exception const &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}

}  // namespace mini_bit
