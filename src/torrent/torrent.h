#ifndef TORRENT_H
#define TORRENT_H

#include "bencode.h"
#include "peer.h"

namespace mini_bit {

constexpr int kBlockSize = 2 << 13;

class Torrent {
public:
  Torrent(Bencode torrent_info);
  void SendTrackerRequest();
  void PeerConnect();

private:
  boost::shared_ptr<boost::asio::io_service> io_service_ptr_;
  std::vector<unsigned char> info_hash_;
  std::string peer_id_;
  std::string info_hash_encoded_;
  std::string announce_;
  std::string host_;
  int port_;
  int file_length_;
  int piece_length_;
  int num_pieces_;
  std::vector<std::vector<unsigned char>> piece_hashes_;
  std::vector<PeerIpPort> peers_;
  Bencode info_;
  std::ofstream out_file_;
};

} // namespace mini_bit

#endif
