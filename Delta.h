#ifndef DELTA_H
#define DELTA_H

#include "Signature.h"
#include <deque>

class Delta {
public:
  Delta(Weaksum &weak, Strongsum &strong, const std::string &in_file_name,
        const std::string &in_sig_file_name,
        const std::string &out_delta_file_name, size_t block_size);
  ~Delta() = default;
  void CreateDelta();
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

  std::vector<char> m_file_data_buffer;
  const char m_block_delimiter{'b'};
  const int m_block_num_field_size{4};

  const char m_data_delimiter{'d'};
  const int m_data_num_field_size{4};

  bool Match(uint32_t weak_sig, uint32_t &block_num);
  void WriteFile(std::ofstream &fout, uint32_t block_num);
  void WriteTail(std::ofstream &fout);
  void WriteData(char data);
};

#endif
