#ifndef SIGNATURE_H
#define SIGNATURE_H

#include "BlockSignature.h"
#include <cstring>
#include <map>
#include <string>
#include <vector>

struct StrongEnumerated {
  StrongEnumerated(const std::vector<char> &v, uint32_t number)
      : m_v(v), m_number(number){};
  ~StrongEnumerated() = default;
  std::vector<char> m_v;
  uint32_t m_number;
};

using SigMap = std::multimap<uint32_t, StrongEnumerated>;

class Signature {
public:
  Signature(Weaksum &weak, Strongsum &strong, const std::string &in_file_name,
            const std::string &out_file_name, size_t block_size);
  Signature(Weaksum &weak, Strongsum &strong, const std::string &in_file_name,
            size_t block_size);
  ~Signature() = default;
  void CreateSignature();
  void ReadSignature(SigMap &mm);

private:
  Weaksum &m_ws;
  Strongsum &m_ss;
  const std::string m_in_file_name;
  const std::string m_out_file_name;
  const size_t m_block_size;
};

#endif
