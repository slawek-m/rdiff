#include "Signature.h"
#include <fstream>
#include <vector>

Signature::Signature(Weaksum &weak, Strongsum &strong,
                     const std::string &in_file_name,
                     const std::string &out_file_name, size_t block_size)
    : m_ws(weak), m_ss(strong), m_in_file_name(in_file_name),
      m_out_file_name(out_file_name), m_block_size(block_size) {}

Signature::Signature(Weaksum &weak, Strongsum &strong,
                     const std::string &in_file_name, size_t block_size)
    : m_ws(weak), m_ss(strong), m_in_file_name(in_file_name),
      m_block_size(block_size) {}

void Signature::CreateSignature() {
  std::ifstream fin(m_in_file_name, std::ifstream::binary);
  std::ofstream fout(m_out_file_name, std::ofstream::binary);

  std::vector<char> input_buffer(m_block_size);
  const size_t signature_size = m_ws.GetSize() + m_ss.GetSize();
  std::vector<char> signature_buffer(signature_size);
  while (!fin.eof()) {
    fin.read(input_buffer.data(), input_buffer.size());
    size_t actual_block_size = fin.gcount();
    if (actual_block_size) {
      BlockSignature bs(m_ws, m_ss, actual_block_size);
      bs.Sign(reinterpret_cast<unsigned char *>(signature_buffer.data()),
              input_buffer.data());
      fout.write(signature_buffer.data(), signature_size);
    }
  }
}

void Signature::ReadSignature(SigMap &mm) {
  std::ifstream fin(m_in_file_name, std::ifstream::binary);

  const size_t signature_size = m_ws.GetSize() + m_ss.GetSize();
  std::vector<char> signature_buffer(signature_size);

  BlockSignature bs(m_ws, m_ss);
  uint32_t sig_num = 0;

  while (!fin.eof()) {
    fin.read(signature_buffer.data(), signature_buffer.size());
    if (fin.gcount()) {
      uint32_t weak_signature;
      std::vector<char> strong_signature;
      bs.Parse(signature_buffer, weak_signature, strong_signature);
      StrongEnumerated se(strong_signature, sig_num);
      mm.emplace(weak_signature, se);
      ++sig_num;
    }
  }
}
