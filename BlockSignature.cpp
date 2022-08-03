#include "BlockSignature.h"

BlockSignature::BlockSignature(Weaksum &weak, Strongsum &strong,
                               size_t block_size)
    : m_weak(weak), m_strong(strong), m_block_size(block_size),
      m_weak_size(weak.GetSize()), m_strong_size(strong.GetSize()){};

void BlockSignature::Sign(unsigned char *out, void const *in) {
  m_strong.Digest(out, in, m_block_size);
  m_weak.Reset();
  m_weak.Update(static_cast<const unsigned char *>(in), m_block_size);
  auto weak_sig = m_weak.Digest();
  memcpy(out + m_strong_size, &weak_sig, m_weak_size);
}

void BlockSignature::Parse(const std::vector<char> &signature_buffer,
                           uint32_t &weak_signature,
                           std::vector<char> &strong_signature) {
  weak_signature = *reinterpret_cast<uint32_t *>(
      const_cast<char *>(signature_buffer.data()) + m_strong_size);
  std::copy(signature_buffer.data(), signature_buffer.data() + m_strong_size,
            std::back_inserter(strong_signature));
}
