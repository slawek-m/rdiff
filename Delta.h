#ifndef DELTA_H
#define DELTA_H

#include "Config.h"
#include "FileIO.h"
#include "Signature.h"
#include <deque>

class Delta {
public:
  Delta(Weaksum &weak, Strongsum &strong, const std::string &in_file_name,
        const std::string &in_sig_file_name,
        const std::string &out_delta_file_name, size_t block_size);
  ~Delta() = default;
  void CreateDelta(bool is_compressed_mode);
  void ParseDelta();

private:
  Weaksum &m_ws;
  Strongsum &m_ss;
  const std::string m_in_file_name;
  const std::string m_in_sig_file_name;
  const std::string m_out_delta_file_name;
  const size_t m_block_size;
  SigMap m_mmap;
  Signature m_sig;
  std::deque<char> m_dq;
  std::vector<char> m_input_buffer;

  const char m_block_delimiter{BLOCK_DEL};
  const char m_compressed_delimiter{COMPRESSED_DEL};
  const int m_block_num_field_size{BLOCK_SIZE};

  const char m_data_delimiter{DATA_DEL};
  const int m_data_num_field_size{DATA_SIZE};

  const char m_empty_delimiter{EMPTY_DEL};

  bool Match(uint32_t weak_sig, uint32_t &block_num);

  bool IsIdentical();
  bool IsInputEmpty();
  void CreateEmpty();
};

#endif
