#include <fstream>
#include <iostream>
#include <vector>
#include <unordered_map>
#include "boost/any.hpp"

class Bencode {
public:
  enum DataType {
    kString = 0,
    kInt = 1,
    kList = 2,
    kDict = 3
  };

  Bencode();
  Bencode(std::string const&);
  Bencode(int const&);
  Bencode(std::vector<Bencode> const&);
  Bencode(std::unordered_map<std::string, Bencode> const&);
  Bencode(DataType dataType);

  DataType getDataType();
  std::string getString();
  int getInt();
  std::vector<Bencode> getList();
  std::unordered_map<std::string, Bencode> getDict();
private:
  DataType dataType_;
  std::string string_;
  int int_;
  std::vector<Bencode> list_;
  std::unordered_map<std::string, Bencode> dict_; 
};

Bencode::Bencode() {}

Bencode::Bencode(std::string const& bString) {
  dataType_ = Bencode::kString;
  string_ = bString;
}

Bencode::Bencode(int const& bInt) {
  dataType_ = Bencode::kInt;
  int_ = bInt;
}

Bencode::Bencode(std::vector<Bencode> const& bList) {
  dataType_ = Bencode::kList;
  list_ = bList;
}

Bencode::Bencode(std::unordered_map<std::string, Bencode> const& bDict) {
  dataType_ = Bencode::kDict;
  dict_ = bDict;
}

Bencode::DataType Bencode::getDataType() {
  return dataType_;
}

std::string Bencode::getString() {
  return string_;  
}

int Bencode::getInt() {
  return int_;
}

std::vector<Bencode> Bencode::getList() {
  return list_;
}

std::unordered_map<std::string, Bencode> Bencode::getDict() {
  return dict_;
}

Bencode decode(std::vector<char>::iterator& begin,
    std::vector<char>::iterator& end) {
  
  switch(*begin) {

    case 'i': {
      begin++;
      std::string bIntString;
      while(*begin != 'e') {
        bIntString += *begin; 
        begin++;
      }
      int bInt = std::stoi(bIntString);
      return Bencode(bInt);
    }

    case 'l': {
      std::vector<Bencode> bList; 
      begin++;
      while(*begin != 'e') {
        Bencode value = decode(begin, end);
        bList.push_back(value);
        begin++;
      }
      return Bencode(bList);
    }

    case 'd': {
      std::unordered_map<std::string, Bencode> bDict;
      begin++;
      while(*begin != 'e') {
        Bencode key = decode(begin, end);
        begin++;
        Bencode value = decode(begin, end);
        bDict[key.getString()] = value;
        begin++;
      }
      return Bencode(bDict);
    }

    default: {
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
        return Bencode(str); 
      }
      return Bencode("error");
    }
  }
}

Bencode decode_torrent(std::vector<char>::iterator& begin,
    std::vector<char>::iterator& end) {
  Bencode torrentDict = decode(begin, end);
  std::string announce = torrentDict.getDict()["announce"].getString();
  int piecesLength = torrentDict.getDict()["info"].getDict()["piece length"].getInt(); 
  std::cout << announce << std::endl;
  std::cout << piecesLength << std::endl;
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

int main() {
  // Read in torrent
  std::ifstream myfile;
  std::string torrent_filename = "ubuntu-18.04.1-desktop-amd64.iso.torrent";
  std::vector<char> buffer;
  load_file(torrent_filename, buffer);
  
  std::vector<char>::iterator begin = buffer.begin();
  std::vector<char>::iterator end = buffer.end();

  Bencode res = decode_torrent(begin, end);
  return 0;
}
