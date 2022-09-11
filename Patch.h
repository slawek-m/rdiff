#ifndef PATCH_H
#define PATCH_H

#include "Config.h"
#include "FileIO.h"
#include <string>
#include <vector>

class Patch {
public:
  Patch(const std::string &in_oryginal_file_name,
        const std::string &in_delta_file_name,
        const std::string &out_recovered_file_name, uint32_t block_size);
  ~Patch() = default;
  void MakePatch();

private:
  const std::string m_in_oryginal_file_name;
  const std::string m_in_delta_file_name;
  const std::string m_out_recovered_file_name;
  const uint32_t m_block_size;
  FileOut m_fout_recovered;
  FileIn m_fin;

  const char m_block_delimiter{BLOCK_DEL};
  const char m_compressed_delimiter{COMPRESSED_DEL};
  const int m_block_num_field_size{BLOCK_SIZE};

  const char m_data_delimiter{DATA_DEL};
  const int m_data_num_field_size{DATA_SIZE};

  const char m_empty_delimiter{EMPTY_DEL};

  size_t ReadBlockFromOryginal(std::vector<char> &data_buffer,
                               uint32_t block_number);
  size_t ReadBlocksFromOryginal(std::vector<char> &data_buffer,
                                uint32_t block_number_first,
                                uint32_t block_number_last);
  bool CopyOryginal();
  void WriteDataToRecovered(const std::vector<char> &data_buffer,
                            size_t data_size);
};

#endif
