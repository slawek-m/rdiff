#include "Patch.h"
#include <vector>

Patch::Patch(const std::string &in_oryginal_file_name,
             const std::string &in_delta_file_name,
             const std::string &out_recovered_file_name, uint32_t block_size)
    : m_in_oryginal_file_name(in_oryginal_file_name),
      m_in_delta_file_name(in_delta_file_name),
      m_out_recovered_file_name(out_recovered_file_name),
      m_block_size(block_size),
      m_fout_recovered(m_out_recovered_file_name, std::ofstream::binary),
      m_fin(m_in_delta_file_name, std::ifstream::binary) {}

size_t Patch::ReadBlockFromOryginal(std::vector<char> &data_buffer,
                                    uint32_t block_number) {
  size_t size = 0;
  FileIn fin(m_in_oryginal_file_name, std::ifstream::binary);

  for (uint32_t cnt = 0; cnt <= block_number; ++cnt) {
    size = 0;
    for (size_t i = 0; (i < m_block_size) && (!fin.Eof()); ++i) {
      fin.Read(data_buffer.data() + i, 1);
      if (fin.Count()) {
        ++size;
      }
    }
  }
  return size;
}

size_t Patch::ReadBlocksFromOryginal(std::vector<char> &blocks_data_buffer,
                                     uint32_t block_number_first,
                                     uint32_t block_number_last) {
  size_t size, total_size = 0;
  for (uint32_t block_number = block_number_first;
       block_number <= block_number_last; ++block_number) {
    std::vector<char> block_buffer(m_block_size);
    size = ReadBlockFromOryginal(block_buffer, block_number);
    std::copy(block_buffer.begin(), block_buffer.begin() + size,
              std::back_inserter(blocks_data_buffer));
    total_size += size;
  }
  return total_size;
}

bool Patch::CopyOryginal() {
  char val;
  FileIn fin(m_in_delta_file_name, std::ifstream::binary);

  fin.Read(&val, 1);
  if (!fin.Count()) {
    FileIn fin_oryginal(m_in_oryginal_file_name, std::ifstream::binary);

    while (!fin_oryginal.Eof()) {
      fin_oryginal.Read(&val, 1);
      if (fin_oryginal.Count()) {
        m_fout_recovered.Write(&val, 1);
      }
    }
    return true;
  }
  return false;
}

void Patch::WriteDataToRecovered(const std::vector<char> &data_buffer,
                                 size_t data_size) {
  m_fout_recovered.Write(data_buffer.data(), data_size);
}

void Patch::MakePatch() {
  if (CopyOryginal()) {
    return;
  }

  char delimiter;
  std::vector<char> data_buffer(m_block_size);

  while (!m_fin.Eof()) {
    m_fin.Read(&delimiter, 1);
    if (m_fin.Count()) {
      if (delimiter == m_block_delimiter) {
        m_fin.Read(data_buffer.data(), m_block_num_field_size);
        uint32_t block_number =
            *reinterpret_cast<uint32_t *>(data_buffer.data());
        size_t block_size = ReadBlockFromOryginal(data_buffer, block_number);
        WriteDataToRecovered(data_buffer, block_size);
      } else if (delimiter == m_data_delimiter) {
        m_fin.Read(data_buffer.data(), m_data_num_field_size);
        uint32_t data_length =
            *reinterpret_cast<uint32_t *>(data_buffer.data());

        uint32_t buffs_number = data_length / m_block_size;
        uint32_t buff_tail_size = data_length % m_block_size;
        while (buffs_number--) {
          m_fin.Read(data_buffer.data(), m_block_size);
          WriteDataToRecovered(data_buffer, m_block_size);
        }
        m_fin.Read(data_buffer.data(), buff_tail_size);
        WriteDataToRecovered(data_buffer, buff_tail_size);
      } else if (delimiter == m_compressed_delimiter) {
        m_fin.Read(data_buffer.data(), m_block_num_field_size);
        uint32_t block_number_first =
            *reinterpret_cast<uint32_t *>(data_buffer.data());

        m_fin.Read(data_buffer.data(), m_block_num_field_size);
        uint32_t block_number_last =
            *reinterpret_cast<uint32_t *>(data_buffer.data());

        std::vector<char> blocks_data_buffer;
        size_t blocks_data_size = ReadBlocksFromOryginal(
            blocks_data_buffer, block_number_first, block_number_last);
        WriteDataToRecovered(blocks_data_buffer, blocks_data_size);
      } else if (delimiter == m_empty_delimiter) {
        return;
      }
    }
  }
}
