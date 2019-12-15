#include "torrent.h"

#include <iomanip>

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/make_shared.hpp>
#include <boost/variant.hpp>
#include <curl/curl.h>
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

std::vector<mini_bit::PeerIpPort> ParsePeers(Bencode peers) {
  std::vector<mini_bit::PeerIpPort> peers_list;
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
  BencodeDict info_dict = boost::get<BencodeDict>(torrent_dict["info"]);
  info_hash_ = GenerateInfoHash(Encode(torrent_dict["info"]));
  info_hash_encoded_ = UrlEncode(info_hash_);
  announce_ = boost::get<std::string>(torrent_dict["announce"]);

  CURLU *h;
  CURLUcode uc;
  char *host;
  char *port;
  h = curl_url();
  uc = curl_url_set(h, CURLUPART_URL, announce_.c_str(), 0);
  uc = curl_url_get(h, CURLUPART_HOST, &host, 0);
  if (!uc) {
    host_ = std::string(host);
    curl_free(host);
  }
  uc = curl_url_get(h, CURLUPART_PORT, &port, 0);
  if (!uc) {
    port_ = std::atoi(port);
    curl_free(port);
  } else {
    port_ = 6969;
  }

  const std::string CurrentClientID = "-MB0001-";
  std::ostringstream os;
  for (int i = 0; i < 12; ++i) {
    int digit = rand() % 10;
    os << digit;
  }
  peer_id_ = CurrentClientID + os.str();
  io_service_ptr_ = boost::make_shared<boost::asio::io_service>();
  file_length_ = boost::get<int>(info_dict["length"]);
  std::cout << "File Length " << file_length_ << std::endl;
  piece_length_ = boost::get<int>(info_dict["piece length"]);
  std::cout << "Piece Length " << piece_length_ << std::endl;
  std::string pieces = boost::get<std::string>(info_dict["pieces"]);
  num_pieces_ = pieces.size() / 20;
  std::cout << "Num Pieces " << num_pieces_ << std::endl;
  for (int i = 0; i < num_pieces_; i += 20) {
    std::string hash_str = pieces.substr(i, 20);
    std::vector<unsigned char> hash(hash_str.begin(), hash_str.end());
    piece_hashes_.push_back(hash);
  }
}

void Torrent::SendTrackerRequest() {
  std::string target = "/announce?info_hash=" + info_hash_encoded_ +
                       "&peer_id=" + peer_id_ +
                       "&port=6889&uploaded=0&downloaded=0&left=" +
                       std::to_string(file_length_) + "&compact=1";

  std::string response_string = GetRequest(host_, port_, target);

  std::vector<char> v(response_string.begin(), response_string.end());
  Bencode ben = DecodeTorrent(v);
  BencodeDict tracker_dict = boost::get<BencodeDict>(ben);
  std::vector<PeerIpPort> peers = ParsePeers(tracker_dict["peers"]);
  peers_ = peers;
}

void Torrent::PeerConnect() {
  std::vector<unsigned char> peer_id(peer_id_.begin(), peer_id_.end());
  Peer peer = Peer(peers_[0], io_service_ptr_, info_hash_, peer_id);
  peer.Connect();
  peer.Handshake();
  PeerMessage bitfield_msg = peer.ReceiveMessage();
  peer.Interested();
  PeerMessage unchoke_msg = peer.ReceiveMessage();
  // for (int piece = 0; piece < num_pieces_; ++piece) {
  //   for (int offset = 0; offset < piece_length_ / kBlockSize;
  //        offset += kBlockSize) {
  //     peer.Request(piece, offset, kBlockSize);
  //   }
  // }
  peer.Request(0x0, 0x0, kBlockSize);
  PeerMessage piece_msg = peer.ReceiveMessage();
}

} // namespace mini_bit
