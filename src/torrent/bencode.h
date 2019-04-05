#include <fstream>
#include <iostream>
#include <vector>
#include <map>
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
  Bencode(std::map<std::string, Bencode> const&);
  Bencode(DataType dataType);

  DataType getDataType();
  std::string getString();
  int getInt();
  std::vector<Bencode> getList();
  std::map<std::string, Bencode> getDict();
private:
  DataType dataType_;
  std::string string_;
  int int_;
  std::vector<Bencode> list_;
  std::map<std::string, Bencode> dict_; 
};

std::string encode(Bencode in);

Bencode decode(std::vector<char>::iterator& begin,
    std::vector<char>::iterator& end);
