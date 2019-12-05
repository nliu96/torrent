#include "bencode.h"
#include "boost/any.hpp"
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

Bencode::Bencode() {}

Bencode::Bencode(std::string const &bString) {
  dataType_ = Bencode::kString;
  string_ = bString;
}

Bencode::Bencode(int const &bInt) {
  dataType_ = Bencode::kInt;
  int_ = bInt;
}

Bencode::Bencode(std::vector<Bencode> const &bList) {
  dataType_ = Bencode::kList;
  list_ = bList;
}

Bencode::Bencode(std::map<std::string, Bencode> const &bDict) {
  dataType_ = Bencode::kDict;
  dict_ = bDict;
}

Bencode::DataType Bencode::getDataType() { return dataType_; }

std::string Bencode::getString() { return string_; }

int Bencode::getInt() { return int_; }

std::vector<Bencode> Bencode::getList() { return list_; }

std::map<std::string, Bencode> Bencode::getDict() { return dict_; }

std::string encode(Bencode in) {
  switch (in.getDataType()) {
  case Bencode::kString: {
    std::string length = std::to_string(in.getString().length());
    return length + ":" + in.getString();
  }

  case Bencode::kInt: {
    std::string integer = std::to_string(in.getInt());
    return "i" + integer + "e";
  }

  case Bencode::kList: {
    std::string listStrings;
    for (size_t i = 0; i < in.getList().size(); i++) {
      listStrings = listStrings + encode(in.getList()[i]);
    }
    return "l" + listStrings + "e";
  }

  case Bencode::kDict: {
    std::string dictStrings;
    for (const auto &dictEntry : in.getDict()) {
      std::string length = std::to_string(dictEntry.first.length());
      dictStrings = dictStrings + length + ":" + dictEntry.first +
                    encode(dictEntry.second);
    }
    return "d" + dictStrings + "e";
  }

  default: {
    return "error";
  }
  }
}

Bencode decode(std::vector<char>::iterator &begin,
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
    std::vector<Bencode> bList;
    begin++;
    while (*begin != 'e') {
      Bencode value = decode(begin, end);
      bList.push_back(value);
      begin++;
    }
    return Bencode(bList);
  }

  case 'd': {
    std::map<std::string, Bencode> bDict;
    begin++;
    while (*begin != 'e') {
      Bencode key = decode(begin, end);
      begin++;
      Bencode value = decode(begin, end);
      std::cout << "Key: " << key.getString() << std::endl;
      std::cout << "Value: " << value.getString() << std::endl;
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
