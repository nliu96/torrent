#ifndef PEER_H
#define PEER_H

#include <utility>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace mini_bit {

using tcp = boost::asio::ip::tcp;
using PeerIpPort = std::pair<boost::asio::ip::address, int>;

class Peer {
public:
  Peer(PeerIpPort peer_ip_port,
       boost::shared_ptr<boost::asio::io_service> io_service,
       std::vector<unsigned char> info_hash,
       std::vector<unsigned char> client_peer_id);

  bool Connect();
  bool Handshake();
  bool ReceiveMessage();

private:
  PeerIpPort peer_ip_port_;
  std::vector<unsigned char> info_hash_;
  std::vector<unsigned char> client_peer_id_;
  std::vector<unsigned char> target_peer_id_;

  tcp::socket socket_;
};

} // namespace mini_bit

#endif
