#ifndef PEER_H
#define PEER_H

#include <utility>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace mini_bit {

using tcp = boost::asio::ip::tcp;
using PeerIpPort = std::pair<boost::asio::ip::address, int>;

enum PeerMessageType {
  kKeepAlive = -1,
  kChoke = 0,
  kUnchoke = 1,
  kInterested = 2,
  kNotInterested = 3,
  kHave = 4,
  kBitfield = 5,
  kRequest = 6,
  kPiece = 7,
  kCancel = 8,
  kPort = 9,
};

struct PeerMessage {
  int len;
  int id;
  std::vector<unsigned char> payload;

  PeerMessage(int len_) : len(len_) {}

  PeerMessage(int len_, int id_) : len(len_), id(id_) {}

  PeerMessage(int len_, int id_, std::vector<unsigned char> payload_)
      : len(len_), id(id_), payload(payload_) {}

  PeerMessageType GetMessageType() {
    if (len == 0) {
      return kKeepAlive;
    } else {
      return static_cast<PeerMessageType>(len);
    }
  }
};

class Peer {
public:
  Peer(PeerIpPort peer_ip_port,
       boost::shared_ptr<boost::asio::io_service> io_service,
       std::vector<unsigned char> info_hash,
       std::vector<unsigned char> client_peer_id);

  bool Connect();
  bool Handshake();
  PeerMessage ReceiveMessage();
  bool Interested();
  bool Request(int piece, int offset, int block_size);

private:
  PeerIpPort peer_ip_port_;
  std::vector<unsigned char> info_hash_;
  std::vector<unsigned char> client_peer_id_;
  std::vector<unsigned char> target_peer_id_;

  tcp::socket socket_;
};

} // namespace mini_bit

#endif
