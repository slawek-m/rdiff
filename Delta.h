#ifndef DELTA_H
#define DELTA_H

#include "FileIO.h"
#include "Signature.h"
#include <deque>

class Delta {
public:
  Delta(Weaksum &weak, Strongsum &strong, const std::string &in_file_name,
        const std::string &in_sig_file_name,
        const std::string &out_delta_file_name, size_t block_size);
  ~Delta() = default;
  void CreateDelta();
  void CreateCompressedDelta();
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
  const char m_compressed_delimiter{'c'};
  const int m_block_num_field_size{4};

  const char m_data_delimiter{'d'};
  const int m_data_num_field_size{4};

  const char m_empty_delimiter{'e'};

  bool Match(uint32_t weak_sig, uint32_t &block_num);
  void WriteFile(FileOut &fout, uint32_t block_num);
  void WriteTail(FileOut &fout);
  void WriteData(char data);
  bool IsIdentical();
  bool IsEmpty();
  void CreateEmpty();

  void WriteCompressedFile(FileOut &fout,
                           const std::pair<uint32_t, uint32_t> &range);
  class CompressionSM {
  public:
    bool AddBlock(uint32_t block_number, std::pair<uint32_t, uint32_t> &range);
    bool Reset(std::pair<uint32_t, uint32_t> &range);

  private:
    uint32_t m_first, m_last;
    uint32_t m_state{0};
  };
  CompressionSM m_csm;
};

#endif
