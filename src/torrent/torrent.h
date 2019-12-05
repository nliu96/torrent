#include "bencode.h"

namespace mini_bit
{

class Torrent
{
public:
  Torrent(Bencode torrent_info);

private:
  std::string announce_;
  std::string host_;
  int port_;
  Bencode info_;
};

} // namespace mini_bit
