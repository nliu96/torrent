#include "torrent.h"
#include <openssl/sha.h>

namespace mini_bit {

Torrent::Torrent(Bencode torrent_info) {
  std::string info_string = encode(torrent_info.getDict()["info"]);
  announce_ = torrent_info.getDict()["announce"].getString();
  char prot[20], host[20], page[20];
  std::cout << "Announce " << announce_.c_str() << std::endl;
  sscanf(announce_.c_str(), "%[^:]://%[^:]:%d/%s", prot, host, &port_, page);
  std::cout << host << " " << port_ << " " << page << std::endl;
  host_ = std::string(host);
};

} // namespace mini_bit
