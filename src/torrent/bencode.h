#ifndef BENCODE_H
#define BENCODE_H

#include <fstream>
#include <iostream>
#include <map>
#include <vector>

#include "boost/variant.hpp"

typedef boost::make_recursive_variant<
    int, std::vector<boost::recursive_variant_>,
    std::map<std::string, boost::recursive_variant_>, std::string>::type
    Bencode;
typedef std::vector<Bencode> BencodeList;
typedef std::map<std::string, Bencode> BencodeDict;

std::string Encode(const Bencode& data);
Bencode DecodeTorrent(std::vector<char>& file_buffer);

#endif
