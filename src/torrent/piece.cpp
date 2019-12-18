#include "piece.h"

#include <openssl/sha.h>

#include "http.h"

namespace mini_bit {

Piece::Piece(int index, int block_size, int piece_length)
    : index_(index), block_size_(block_size) {
  data_.reserve(piece_length / block_size);
}

bool Piece::InsertBlock(int offset, const std::vector<unsigned char> &data) {
  int block_index = offset / block_size_;
  data_[block_index] = data;
  return true;
}

bool Piece::ValidateHash(std::vector<unsigned char> hash) {
  unsigned char obuf[20];

  return true;
}

} // namespace mini_bit
