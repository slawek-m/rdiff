#include "Patch.h"
#include <fstream>
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
  std::ifstream fin(m_in_oryginal_file_name, std::ifstream::binary);

  for (uint32_t cnt = 0; cnt <= block_number; ++cnt) {
    size = 0;
    for (size_t i = 0; (i < m_block_size) && (!fin.eof()); ++i) {
      fin.read(data_buffer.data() + i, 1);
      if (fin.gcount()) {
        ++size;
      }
    }
  }
  return size;
}

bool Patch::CopyOryginal() {
  char val;
  std::ifstream fin(m_in_delta_file_name, std::ifstream::binary);

  fin.read(&val, 1);
  if (!fin.gcount()) {
    std::ifstream fin_oryginal(m_in_oryginal_file_name, std::ifstream::binary);

    while (!fin_oryginal.eof()) {
      fin_oryginal.read(&val, 1);
      if (fin_oryginal.gcount()) {
        m_fout_recovered.write(&val, 1);
      }
    }
    return true;
  }
  return false;
}

void Patch::WriteDataToRecovered(const std::vector<char> &data_buffer,
                                 size_t data_size) {
  m_fout_recovered.write(data_buffer.data(), data_size);
}

void Patch::MakePatch() {
  char delimiter;
  std::vector<char> data_buffer(m_block_size);

  if (CopyOryginal()) {
    return;
  }

  while (!m_fin.eof()) {
    m_fin.read(&delimiter, 1);
    if (m_fin.gcount()) {
      if (delimiter == m_block_delimiter) {
        m_fin.read(data_buffer.data(), m_block_num_field_size);
        uint32_t block_number =
            *reinterpret_cast<uint32_t *>(data_buffer.data());
        size_t block_size = ReadBlockFromOryginal(data_buffer, block_number);
        WriteDataToRecovered(data_buffer, block_size);
      } else if (delimiter == m_data_delimiter) {
        m_fin.read(data_buffer.data(), m_data_num_field_size);
        uint32_t data_length =
            *reinterpret_cast<uint32_t *>(data_buffer.data());

        uint32_t buffs_number = data_length / m_block_size;
        uint32_t buff_tail_size = data_length % m_block_size;
        while (buffs_number--) {
          m_fin.read(data_buffer.data(), m_block_size);
          WriteDataToRecovered(data_buffer, m_block_size);
        }
        m_fin.read(data_buffer.data(), buff_tail_size);
        WriteDataToRecovered(data_buffer, buff_tail_size);
      }
    }
  }
}
