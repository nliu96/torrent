#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "torrent/peer_protocol.h"
#include "torrent/torrent.h"

void load_file(std::string filename, std::vector<char> &buffer) {
  std::ifstream file(filename, std::ios::binary);
  std::streampos fileSize;
  file.unsetf(std::ios::skipws);
  file.seekg(0, std::ios::end);
  fileSize = file.tellg();
  file.seekg(0, std::ios::beg);
  buffer.reserve(fileSize);
  buffer.insert(buffer.begin(), std::istream_iterator<char>(file),
                std::istream_iterator<char>());
}

int main(int argc, char **argv) {
  std::ifstream myfile;
  std::string torrentFilename = argv[1];
  std::vector<char> file_buffer;
  load_file(torrentFilename, file_buffer);

  Bencode file_bencode = DecodeTorrent(file_buffer);
  mini_bit::Torrent torrent = mini_bit::Torrent(file_bencode);
  torrent.SendTrackerRequest();
  torrent.PeerConnect();

  return EXIT_SUCCESS;
}
