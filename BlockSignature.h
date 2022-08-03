#ifndef BLOCKSIGNATURE_H
#define BLOCKSIGNATURE_H

#include "Strongsum.h"
#include "Weaksum.h"
#include <vector>

class BlockSignature {
public:
  BlockSignature(Weaksum &weak, Strongsum &strong, size_t block_size = 0);
  ~BlockSignature() = default;
  void Sign(unsigned char *out, void const *in);
  void Parse(const std::vector<char> &signature_buffer,
             uint32_t &weak_signature, std::vector<char> &strong_signature);

private:
  Weaksum &m_weak;
  Strongsum &m_strong;
  size_t m_block_size;
  size_t m_weak_size;
  size_t m_strong_size;
};

#endif
