#include "WriterDelta.h"
#include <iostream>

void WriterDelta::WriteDataImpl(char data) {
  m_file_data_buffer.push_back(data);
}

void WriterDelta::WriteFileImpl(FileOut &fout, uint32_t block_num) {
  const size_t out_data_size = m_file_data_buffer.size();
  if (out_data_size) {
    fout.Write(&m_data_delimiter, 1);

    fout.Write(reinterpret_cast<const char *>(&out_data_size),
               m_data_num_field_size);
    fout.Write(m_file_data_buffer.data(), out_data_size);
    m_file_data_buffer.clear();
  }

  fout.Write(&m_block_delimiter, 1);
  fout.Write(reinterpret_cast<char *>(&block_num), m_block_num_field_size);
}

void WriterDelta::WriteCompressedFileImpl(
    FileOut &fout, const std::pair<uint32_t, uint32_t> &range) {
  const size_t out_data_size = m_file_data_buffer.size();
  if (out_data_size) {
    fout.Write(&m_data_delimiter, 1);

    fout.Write(reinterpret_cast<const char *>(&out_data_size),
               m_data_num_field_size);
    fout.Write(m_file_data_buffer.data(), out_data_size);
    m_file_data_buffer.clear();
  }

  if (range.second > range.first) {
    fout.Write(&m_compressed_delimiter, 1);
    fout.Write(reinterpret_cast<char *>(const_cast<uint32_t *>(&range.first)),
               m_block_num_field_size);
    fout.Write(reinterpret_cast<char *>(const_cast<uint32_t *>(&range.second)),
               m_block_num_field_size);
  } else if (range.second == range.first) {
    fout.Write(&m_block_delimiter, 1);
    fout.Write(reinterpret_cast<char *>(const_cast<uint32_t *>(&range.first)),
               m_block_num_field_size);
  } else {
    std::cerr << "range error" << std::endl;
  }
}

void WriterDelta::WriteTailImpl(FileOut &fout) {
  const size_t out_data_size = m_file_data_buffer.size();
  if (out_data_size) {
    fout.Write(&m_data_delimiter, 1);

    fout.Write(reinterpret_cast<const char *>(&out_data_size),
               m_data_num_field_size);
    fout.Write(m_file_data_buffer.data(), out_data_size);
    m_file_data_buffer.clear();
  }
}

bool WriterDeltaCompressed::CompressionSM::AddBlock(
    uint32_t block_number, std::pair<uint32_t, uint32_t> &range) {
  bool ret = false;
  if (m_state == 0) {
    m_first = block_number;
    m_state = 1;
  } else if (m_state == 1) {
    if (block_number - m_first == 1) {
      m_last = block_number;
      m_state = 2;
    } else {
      ret = true;
      range.first = m_first;
      range.second = m_first;

      m_first = block_number;
      m_state = 1;
    }
  } else if (m_state == 2) {
    if (block_number - m_last == 1) {
      m_last = block_number;
      m_state = 2;
    } else {
      ret = true;
      range.first = m_first;
      range.second = m_last;

      m_first = block_number;
      m_state = 1;
    }
  }
  return ret;
}

bool WriterDeltaCompressed::CompressionSM::Reset(
    std::pair<uint32_t, uint32_t> &range) {
  bool ret = false;
  if (m_state == 0) {
    m_state = 0;
  } else if (m_state == 1) {
    ret = true;
    range.first = m_first;
    range.second = m_first;

    m_state = 0;
  } else if (m_state == 2) {
    ret = true;
    range.first = m_first;
    range.second = m_last;

    m_state = 0;
  }
  return ret;
}
