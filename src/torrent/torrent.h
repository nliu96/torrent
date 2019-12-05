#ifndef TORRENT_H
#define TORRENT_H

#include "bencode.h"

namespace mini_bit
{

class Torrent
{
public:
  Torrent(Bencode torrent_info);
  void GetTracker();

private:
  std::string info_hash_;
  std::string announce_;
  std::string host_;
  int port_;
  Bencode info_;
};

} // namespace mini_bit

#endif
