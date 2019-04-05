#include "torrent/bencode.h"

namespace mini_bit
{

class Torrent
{
public:
  Torrent(Bencode torrent_info);

private:
  std::string announce_;
  Bencode info_;
}

} // namespace mini_bit