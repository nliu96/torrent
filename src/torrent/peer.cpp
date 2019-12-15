#include "peer.h"

#include <iostream>

#include <boost/asio.hpp>

#include "peer_protocol.h"

namespace mini_bit {

Peer::Peer(PeerIpPort peer_ip_port,
           boost::shared_ptr<boost::asio::io_service> io_service,
           std::vector<unsigned char> info_hash,
           std::vector<unsigned char> client_peer_id)
    : peer_ip_port_(peer_ip_port), info_hash_(info_hash),
      client_peer_id_(client_peer_id), socket_(*io_service) {}

bool Peer::Connect() {
  tcp::endpoint ep(peer_ip_port_.first, peer_ip_port_.second);
  socket_.connect(ep);
  std::cout << "Connected to " << peer_ip_port_.first << ":"
            << peer_ip_port_.second << std::endl;
  return true;
}

bool Peer::Handshake() {
  std::vector<unsigned char> msg = MakeHandshake(info_hash_, client_peer_id_);
  boost::asio::write(socket_, boost::asio::buffer(msg));
  std::vector<unsigned char> handshake_response(68);
  boost::asio::read(socket_, boost::asio::buffer(handshake_response));
  target_peer_id_ = ReadHandshake(handshake_response);
  return true;
}

PeerMessage Peer::ReceiveMessage() {
  std::vector<unsigned char> length_prefix(4);
  boost::asio::read(socket_, boost::asio::buffer(length_prefix));

  int len = 0;
  for (auto c : length_prefix) {
    len = (len << 8) + ((unsigned int)c);
  }
  std::cout << "Length " << len << std::endl;

  // keep-alive message
  if (len == 0) {
    return PeerMessage(len);
  }

  std::vector<unsigned char> msg_id(1);
  boost::asio::read(socket_, boost::asio::buffer(msg_id));

  if (len == 1) {
    return PeerMessage(len, msg_id[0]);
  }

  std::vector<unsigned char> payload(len - 1);
  boost::asio::read(socket_, boost::asio::buffer(payload));

  std::cout << "Message ID " << (unsigned int)msg_id[0] << std::endl;

  return PeerMessage(len, msg_id[0], payload);
}

bool Peer::Interested() {
  std::vector<unsigned char> msg = MakeInterested();
  boost::asio::write(socket_, boost::asio::buffer(msg));
  return true;
}

bool Peer::Request(int piece, int offset, int block_size) {
  std::vector<unsigned char> msg = MakeRequest(piece, offset, block_size);
  boost::asio::write(socket_, boost::asio::buffer(msg));
  return true;
}

} // namespace mini_bit
