#include "peer_protocol.h"

#include <iostream>

namespace mini_bit {

std::vector<unsigned char> MakeHandShake(std::vector<unsigned char> info_hash,
                                         std::vector<unsigned char> peer_id) {
  std::vector<unsigned char> msg;
  // pstrlen
  msg.push_back(static_cast<char>(0x13));
  // pstr
  std::string pstr = "BitTorrent protocol";
  std::copy(pstr.begin(), pstr.end(), std::back_inserter(msg));
  // // reserved
  msg.insert(msg.end(), 8, 0x00);
  // // info_hash
  msg.insert(msg.end(), info_hash.begin(), info_hash.end());
  // // peer_id
  msg.insert(msg.end(), peer_id.begin(), peer_id.end());

  std::cout << std::endl;

  return msg;
}

std::vector<unsigned char> ReadHandShake(std::vector<unsigned char> handshake) {
  std::cout << (unsigned int)handshake[0] << std::endl;
  std::vector<unsigned char> peer_id(handshake.begin() + 48, handshake.end());
  return peer_id;
}

} // namespace mini_bit
