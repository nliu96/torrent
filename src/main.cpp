#include <fstream>
#include <iostream>
#include <vector>
#include <unordered_map>
#include "boost/any.hpp"

std::string decode(std::vector<char>::iterator& begin,
    std::vector<char>::iterator& end) {
  switch(*begin) {
    case 'i': {
      return "i";
    }
    case 'd': {
      std::unordered_map<std::string, boost::any> dict;
      while(*(++begin) != 'e') {
        std::cout << *begin << std::endl;
        std::string key = decode(begin, end);
        std::string value = decode(begin, end);
        dict[key] = value;
        std::cout << key << std::endl;
      }
      return "success";
    }
    default:
      // Byte string
      if (std::isdigit(*begin)) {
        std::string len_str;
        while (*begin != ':') {
          len_str += *begin;
          begin++;
        }
        int len = std::stoi(len_str);
        std::string str;
        for (int i = 0; i < len; i++) {
          str += *(++begin); 
        }
        return str; 
      } else {
        return "wtf";
      }
  }
  return "1";
}

void load_file(std::string filename, std::vector<char>& buffer) {
  std::ifstream file(filename, std::ios::binary);
  std::streampos fileSize;
  file.seekg(0, std::ios::end);
  fileSize = file.tellg();
  file.seekg(0, std::ios::beg);
  buffer.reserve(fileSize);
  buffer.insert(buffer.begin(),
                std::istream_iterator<char>(file),
                std::istream_iterator<char>());
}

int main() {
  // Read in torrent
  std::ifstream myfile;
  std::string torrent_filename = "ubuntu-18.04.1-desktop-amd64.iso.torrent";
  std::vector<char> buffer;
  load_file(torrent_filename, buffer);
  
  std::vector<char>::iterator begin = buffer.begin();
  std::vector<char>::iterator end = buffer.end();

  std::string res = decode(begin, end);

  std::cout << res << std::endl;

  return 0;
}
