#include <fstream>
#include <iostream>
#include <vector>
#include <unordered_map>
#include "boost/any.hpp"
#include "bencode.h"

Bencode decode_torrent(std::vector<char>::iterator& begin,
    std::vector<char>::iterator& end) {
  Bencode torrentDict = decode(begin, end);
  std::string announce = torrentDict.getDict()["announce"].getString();
  int piecesLength = torrentDict.getDict()["info"].getDict()["piece length"].getInt();
  return torrentDict;
}

void load_file(std::string filename, std::vector<char>& buffer) {
  std::ifstream file(filename, std::ios::binary);
  std::streampos fileSize;
  file.unsetf(std::ios::skipws);
  file.seekg(0, std::ios::end);
  fileSize = file.tellg();
  file.seekg(0, std::ios::beg);
  buffer.reserve(fileSize);
  buffer.insert(buffer.begin(),
                std::istream_iterator<char>(file),
                std::istream_iterator<char>());
}

int main(int argc, char** argv) {
  // Read in torrent
  std::ifstream myfile;
  std::string torrent_filename = argv[1];
  std::vector<char> buffer;
  load_file(torrent_filename, buffer);
  
  std::vector<char>::iterator begin = buffer.begin();
  std::vector<char>::iterator end = buffer.end();

  Bencode res = decode_torrent(begin, end);
  return 0;
}
