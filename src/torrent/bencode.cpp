#include "bencode.h"

#include <fstream>
#include <iostream>
#include <map>
#include <vector>

namespace {

class Visitor : public boost::static_visitor<std::string> {
public:
  std::string operator()(int i) const {
    std::string integer = std::to_string(i);
    return "i" + integer + "e";
  }

  std::string operator()(BencodeList l) const {
    std::string listStrings;
    for (size_t i = 0; i < l.size(); i++) {
      listStrings = listStrings + boost::apply_visitor(*this, l[i]);
    }
    return "l" + listStrings + "e";
  }

  std::string operator()(BencodeDict d) const {
    std::string dictStrings;
    for (const auto &dictEntry : d) {
      std::string length = std::to_string(dictEntry.first.length());
      dictStrings = dictStrings + length + ":" + dictEntry.first +
                    boost::apply_visitor(*this, dictEntry.second);
    }
    return "d" + dictStrings + "e";
  }

  std::string operator()(std::string s) const {
    std::string length = std::to_string(s.length());
    return length + ":" + s;
  }
};

Bencode Decode(std::vector<char>::iterator &begin,
               std::vector<char>::iterator &end) {
  switch (*begin) {
  case 'i': {
    begin++;
    std::string bIntString;
    while (*begin != 'e') {
      bIntString += *begin;
      begin++;
    }
    int bInt = std::stoi(bIntString);
    return Bencode(bInt);
  }

  case 'l': {
    BencodeList bList;
    begin++;
    while (*begin != 'e') {
      Bencode value = Decode(begin, end);
      bList.push_back(value);
      begin++;
    }
    return Bencode(bList);
  }

  case 'd': {
    std::map<std::string, Bencode> bDict;
    begin++;
    while (*begin != 'e') {
      Bencode key = Decode(begin, end);
      begin++;
      Bencode value = Decode(begin, end);
      bDict[boost::get<std::string>(key)] = value;
      begin++;
    }
    Bencode decoded(bDict);
    return decoded;
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
    return Bencode("Error");
  }
  }
}

} // namespace

std::string Encode(const Bencode &data) {
  return boost::apply_visitor(Visitor(), data);
}

Bencode DecodeTorrent(std::vector<char> &file_buffer) {
  std::vector<char>::iterator file_begin = file_buffer.begin();
  std::vector<char>::iterator file_end = file_buffer.end();
  return Decode(file_begin, file_end);
}
