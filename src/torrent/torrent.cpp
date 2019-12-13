#include "torrent.h"

#include <iomanip>

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/make_shared.hpp>
#include <boost/variant.hpp>
#include <openssl/sha.h>

#include "http.h"
#include "peer.h"

namespace net = boost::asio;

namespace {

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

std::vector<unsigned char> GenerateInfoHash(const std::string &info) {
  unsigned char obuf[20];
  unsigned char val[info.length() + 1];
  std::memcpy(val, info.c_str(), info.length());
  SHA1(val, sizeof(val) - 1, obuf);
  std::vector<unsigned char> info_hash(obuf, obuf + 20);
  return info_hash;
}

std::vector<std::pair<net::ip::address, int>> ParsePeers(Bencode peers) {
  std::vector<std::pair<net::ip::address, int>> peers_list;
  std::string peers_str = boost::get<std::string>(peers);
  std::vector<unsigned char> peers_bytes(peers_str.begin(), peers_str.end());
  for (size_t i = 0; i < peers_bytes.size(); i += 6) {
    std::string addr_byte_0 = std::to_string((int)peers_bytes[i + 0]);
    std::string addr_byte_1 = std::to_string((int)peers_bytes[i + 1]);
    std::string addr_byte_2 = std::to_string((int)peers_bytes[i + 2]);
    std::string addr_byte_3 = std::to_string((int)peers_bytes[i + 3]);
    std::string peer_address =
        addr_byte_0 + "." + addr_byte_1 + "." + addr_byte_2 + "." + addr_byte_3;
    net::ip::address ip_address = net::ip::address::from_string(peer_address);
    int port_byte_0 = (int)peers_bytes[i + 4];
    int port_byte_1 = (int)peers_bytes[i + 5];
    int port = (port_byte_0 << 8) + port_byte_1;
    peers_list.push_back(std::make_pair(ip_address, port));
  }
  std::cout << "Found " << peers_list.size() << " peers." << std::endl;
  return peers_list;
}

} // namespace

namespace mini_bit {

Torrent::Torrent(Bencode torrent_info) {
  BencodeDict torrent_dict = boost::get<BencodeDict>(torrent_info);
  info_hash_ = GenerateInfoHash(Encode(torrent_dict["info"]));
  info_hash_encoded_ = UrlEncode(info_hash_);
  announce_ = boost::get<std::string>(torrent_dict["announce"]);
  char prot[20], host[20], page[20];
  sscanf(announce_.c_str(), "%[^:]://%[^:]:%d/%s", prot, host, &port_, page);
  host_ = std::string(host);

  const std::string CurrentClientID = "-MB0001-";
  std::ostringstream os;
  for (int i = 0; i < 12; ++i) {
    int digit = rand() % 10;
    os << digit;
  }
  peer_id_ = CurrentClientID + os.str();
}

void Torrent::SendTrackerRequest() {
  std::string target =
      "/announce?info_hash=" + info_hash_encoded_ + "&peer_id=" + peer_id_ +
      "&port=6889&uploaded=0&downloaded=0&left=2097152000&compact=1";

  std::cout << target << std::endl;

  std::string response_string = GetRequest(host_, port_, target);

  std::vector<char> v(response_string.begin(), response_string.end());
  Bencode ben = DecodeTorrent(v);
  BencodeDict tracker_dict = boost::get<BencodeDict>(ben);
  std::vector<PeerIpPort> peers = ParsePeers(tracker_dict["peers"]);
  peers_ = peers;
}

void Torrent::PeerConnect() {
  boost::shared_ptr<boost::asio::io_service> io_service_ptr =
      boost::make_shared<boost::asio::io_service>();

  std::vector<unsigned char> peer_id(peer_id_.begin(), peer_id_.end());
  Peer peer = Peer(peers_[0], io_service_ptr, info_hash_, peer_id);
  peer.Connect();
  peer.Handshake();
  peer.ReceiveMessage();
  peer.Interested();
  peer.ReceiveMessage();
}

} // namespace mini_bit
