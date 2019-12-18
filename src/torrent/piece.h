#ifndef PIECE_H
#define PIECE_H

#include <vector>

namespace mini_bit {

class Piece {
public:
  Piece(int index, int block_size, int piece_length);
  bool InsertBlock(int offset, const std::vector<unsigned char> &data);
  bool ValidateHash(std::vector<unsigned char> hash);

private:
  int index_;
  int block_size_;

  std::vector<std::vector<unsigned char>> data_;
};

} // namespace mini_bit

#endif
