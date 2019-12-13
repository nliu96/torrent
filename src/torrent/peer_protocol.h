#ifndef PEER_PROTOCOL_H
#define PEER_PROTOCOL_H

#include "boost/asio/buffer.hpp"

namespace mini_bit {

std::vector<unsigned char> MakeHandshake(std::vector<unsigned char> info_hash,
                                         std::vector<unsigned char> peer_id);

std::vector<unsigned char> MakeInterested();

std::vector<unsigned char> ReadHandshake(std::vector<unsigned char> handshake);

} // namespace mini_bit

#endif
