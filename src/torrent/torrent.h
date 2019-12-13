#ifndef TORRENT_H
#define TORRENT_H

#include "bencode.h"
#include "peer.h"

namespace mini_bit {

class Torrent {
public:
  Torrent(Bencode torrent_info);
  void SendTrackerRequest();
  void PeerConnect();

private:
  std::vector<unsigned char> info_hash_;
  std::string peer_id_;
  std::string info_hash_encoded_;
  std::string announce_;
  std::string host_;
  int port_;
  std::vector<PeerIpPort> peers_;
  Bencode info_;
};

} // namespace mini_bit

#endif
