#include "Delta.h"
#include <fstream>
#include <iostream>

//#define DEBUG_ENABLE 1
//#define VERBOSE_ENABLE 1

Delta::Delta(Weaksum &weak, Strongsum &strong, const std::string &in_file_name,
             const std::string &in_sig_file_name,
             const std::string &out_delta_file_name, size_t block_size)
    : m_ws(weak), m_ss(strong), m_in_file_name(in_file_name),
      m_in_sig_file_name(in_sig_file_name),
      m_out_delta_file_name(out_delta_file_name), m_block_size(block_size),
      m_sig(m_ws, m_ss, m_in_sig_file_name, m_block_size),
      m_input_buffer(m_block_size) {
  m_sig.ReadSignature(m_mmap);
}

void Delta::CreateDelta() {

  if (IsIdentical()) {
    std::ofstream fout(m_out_delta_file_name, std::ofstream::binary);
    return;
  }

  if (IsEmpty()) {
    CreateEmpty();
    return;
  }

  std::ifstream fin(m_in_file_name, std::ifstream::binary);
  std::ofstream fout(m_out_delta_file_name, std::ofstream::binary);

  char buff;
  char front;
  uint32_t weak_sig;
  uint32_t block_num;

  while (!fin.eof()) {
    fin.read(&buff, 1);
    if (fin.gcount()) {
      m_dq.push_back(buff);
      auto size = m_dq.size();
      if (size == 1) {
        m_ws.Reset();
        m_ws.Update(reinterpret_cast<const unsigned char *>(&buff), size);
      } else if ((size > 1) && (size < m_block_size)) {
        m_ws.Rollin(buff);
      } else if (size == m_block_size) {
        m_ws.Rollin(buff);
        weak_sig = m_ws.Digest();
#ifdef DEBUG_ENABLE
        printf("match 1\n");
        for (const auto &a : m_dq) {
          printf(" %x", a);
        }
        printf("\n");
#endif

        if (Match(weak_sig, block_num)) {
#ifdef DEBUG_ENABLE
          printf("%x:", weak_sig);
          printf(" :");
          for (const auto &a : m_dq) {
            printf(" %x", a);
          }
          printf("\n\n");
#endif
          m_dq.clear();

#ifdef VERBOSE_ENABLE
          printf("block num =====>: %d\n", block_num);
#endif
          WriteFile(fout, block_num);
        }
      } else if (size > m_block_size) {
#ifdef DEBUG_ENABLE
        printf("match 2\n");
#endif
        front = m_dq.front();
        m_dq.pop_front();
        m_ws.Rotate(front, buff);
        weak_sig = m_ws.Digest();

#ifdef VERBOSE_ENABLE
        printf("val =====>: %c, %x\n", front, front);
#endif
        WriteData(front);

        if (Match(weak_sig, block_num)) {
#ifdef DEBUG_ENABLE
          printf("%x:", weak_sig);
          printf(" :");
          for (const auto &a : m_dq) {
            printf(" %x", a);
          }
          printf("\n\n");
#endif
          m_dq.clear();
#ifdef VERBOSE_ENABLE
          printf("block num =====>: %d\n", block_num);
#endif
          WriteFile(fout, block_num);
        }
      }
    }
  }

#ifdef DEBUG_ENABLE
  printf("\ntail\n");
  for (const auto &a : m_dq) {
    printf(" %x", a);
  }
  printf("\n");
#endif
  while (m_dq.size()) {
    weak_sig = m_ws.Digest();
    if (Match(weak_sig, block_num)) {
      m_dq.clear();
#ifdef VERBOSE_ENABLE
      printf("block num =====>: %d\n", block_num);
#endif
      WriteFile(fout, block_num);
    } else {
      front = m_dq.front();
      m_dq.pop_front();
      m_ws.Rollout(front);

#ifdef VERBOSE_ENABLE
      printf("val =====>: %c, %x\n", front, front);
#endif
      WriteData(front);
    }
  }
  WriteTail(fout);
}

bool Delta::Match(uint32_t weak_sig, uint32_t &block_num) {
  bool res = false;
  std::vector<unsigned char> strong_sig(m_ss.GetSize());
  std::copy(m_dq.begin(), m_dq.end(), m_input_buffer.begin());

  std::pair<SigMap::iterator, SigMap::iterator> range;
  range = m_mmap.equal_range(weak_sig);

  for (auto it = range.first; it != range.second; ++it) {
    m_ss.Digest(strong_sig.data(), m_input_buffer.data(), m_dq.size());
    if (memcmp(strong_sig.data(), it->second.m_v.data(), m_ss.GetSize()) == 0) {
      res = true;
      block_num = it->second.m_number;
#ifdef DEBUG_ENABLE
      printf("%x:", weak_sig);
      for (int i = 0; i < 16; ++i) {
        printf(" %x", strong_sig[i]);
      }
      printf(" :%d", block_num);
      printf("\n");
#endif

      break;
    }
  }
  return res;
}

void Delta::CreateCompressedDelta() {
  if (IsIdentical()) {
    std::ofstream fout(m_out_delta_file_name, std::ofstream::binary);
    return;
  }

  if (IsEmpty()) {
    CreateEmpty();
    return;
  }

  std::ifstream fin(m_in_file_name, std::ifstream::binary);
  std::ofstream fout(m_out_delta_file_name, std::ofstream::binary);

  char buff;
  char front;
  uint32_t weak_sig;
  uint32_t block_num;
  std::pair<uint32_t, uint32_t> range;

  while (!fin.eof()) {
    fin.read(&buff, 1);
    if (fin.gcount()) {
      m_dq.push_back(buff);
      auto size = m_dq.size();
      if (size == 1) {
        m_ws.Reset();
        m_ws.Update(reinterpret_cast<const unsigned char *>(&buff), size);
      } else if ((size > 1) && (size < m_block_size)) {
        m_ws.Rollin(buff);
      } else if (size == m_block_size) {
        m_ws.Rollin(buff);
        weak_sig = m_ws.Digest();

        if (Match(weak_sig, block_num)) {
          m_dq.clear();

#ifdef VERBOSE_ENABLE
          printf("1 block num =====>: %d\n", block_num);
#endif
          if (m_csm.AddBlock(block_num, range)) {
            WriteCompressedFile(fout, range);
#ifdef VERBOSE_ENABLE
            printf("1 range: %d, %d:%d\n", range.first != range.second,
                   range.first, range.second);
#endif
          }
        }
      } else if (size > m_block_size) {
        front = m_dq.front();
        m_dq.pop_front();
        m_ws.Rotate(front, buff);
        weak_sig = m_ws.Digest();
#ifdef VERBOSE_ENABLE
        printf("2 val =====>: %c, %x\n", front, front);
#endif
        if (m_csm.Reset(range)) {
          WriteCompressedFile(fout, range);
#ifdef VERBOSE_ENABLE
          printf("2 range: %d, %d:%d\n", range.first != range.second,
                 range.first, range.second);
#endif
        }
        WriteData(front);

        if (Match(weak_sig, block_num)) {
          m_dq.clear();
#ifdef VERBOSE_ENABLE
          printf("2 block num =====>: %d\n", block_num);
#endif
          if (m_csm.AddBlock(block_num, range)) {
            WriteCompressedFile(fout, range);
#ifdef VERBOSE_ENABLE
            printf("3 range: %d, %d:%d\n", range.first != range.second,
                   range.first, range.second);
#endif
          }
        }
      }
    }
  }

  while (m_dq.size()) {
    weak_sig = m_ws.Digest();
    if (Match(weak_sig, block_num)) {
      m_dq.clear();
#ifdef VERBOSE_ENABLE
      printf("t block num =====>: %d\n", block_num);
#endif
      if (m_csm.AddBlock(block_num, range)) {
        WriteCompressedFile(fout, range);
#ifdef VERBOSE_ENABLE
        printf("4 range: %d, %d:%d\n", range.first != range.second, range.first,
               range.second);
#endif
      }
    } else {
      front = m_dq.front();
      m_dq.pop_front();
      m_ws.Rollout(front);

#ifdef VERBOSE_ENABLE
      printf("t val =====>: %c, %x\n", front, front);
#endif
      if (m_csm.Reset(range)) {
        WriteCompressedFile(fout, range);
#ifdef VERBOSE_ENABLE
        printf("5 range: %d, %d:%d\n", range.first != range.second, range.first,
               range.second);
#endif
      }
      WriteData(front);
    }
  }
  if (m_csm.Reset(range)) {
    WriteCompressedFile(fout, range);
#ifdef VERBOSE_ENABLE
    printf("6 range: %d, %d:%d\n", range.first != range.second, range.first,
           range.second);
#endif
  }
  WriteTail(fout);
}

void Delta::WriteData(char data) { m_file_data_buffer.push_back(data); }

void Delta::WriteFile(std::ofstream &fout, uint32_t block_num) {
  const size_t out_data_size = m_file_data_buffer.size();
  if (out_data_size) {
    fout.write(&m_data_delimiter, 1);

    fout.write(reinterpret_cast<const char *>(&out_data_size),
               m_data_num_field_size);
    fout.write(m_file_data_buffer.data(), out_data_size);
    m_file_data_buffer.clear();
  }

  fout.write(&m_block_delimiter, 1);
  fout.write(reinterpret_cast<char *>(&block_num), m_block_num_field_size);
}

void Delta::WriteCompressedFile(std::ofstream &fout,
                                const std::pair<uint32_t, uint32_t> &range) {
  const size_t out_data_size = m_file_data_buffer.size();
  if (out_data_size) {
    fout.write(&m_data_delimiter, 1);

    fout.write(reinterpret_cast<const char *>(&out_data_size),
               m_data_num_field_size);
    fout.write(m_file_data_buffer.data(), out_data_size);
    m_file_data_buffer.clear();
  }

  if (range.second > range.first) {
    fout.write(&m_compressed_delimiter, 1);
    fout.write(reinterpret_cast<char *>(const_cast<uint32_t *>(&range.first)),
               m_block_num_field_size);
    fout.write(reinterpret_cast<char *>(const_cast<uint32_t *>(&range.second)),
               m_block_num_field_size);
  } else if (range.second == range.first) {
    fout.write(&m_block_delimiter, 1);
    fout.write(reinterpret_cast<char *>(const_cast<uint32_t *>(&range.first)),
               m_block_num_field_size);
  } else {
    std::cerr << "range error" << std::endl;
  }
}

void Delta::WriteTail(std::ofstream &fout) {
  const size_t out_data_size = m_file_data_buffer.size();
  if (out_data_size) {
    fout.write(&m_data_delimiter, 1);

    fout.write(reinterpret_cast<const char *>(&out_data_size),
               m_data_num_field_size);
    fout.write(m_file_data_buffer.data(), out_data_size);
    m_file_data_buffer.clear();
  }
}

bool Delta::IsIdentical() {
  std::ifstream fin(m_in_file_name, std::ifstream::binary);
  std::ifstream fsig(m_in_sig_file_name, std::ifstream::binary);

  std::vector<char> input_buffer(m_block_size);
  const size_t signature_size = m_ws.GetSize() + m_ss.GetSize();
  std::vector<char> computed_signature_buffer(signature_size);
  std::vector<char> read_signature_buffer(signature_size);

  while (!fin.eof() && !fsig.eof()) {
    fin.read(input_buffer.data(), input_buffer.size());
    size_t actual_block_size = fin.gcount();

    fsig.read(read_signature_buffer.data(), read_signature_buffer.size());
    size_t actual_signature_size = fsig.gcount();

    if (((actual_block_size > 0) && (actual_signature_size == 0)) ||
        ((actual_block_size == 0) && (actual_signature_size > 0))) {
      return false;
    } else if ((actual_block_size > 0) &&
               (actual_signature_size == signature_size)) {
      BlockSignature bs(m_ws, m_ss, actual_block_size);
      bs.Sign(
          reinterpret_cast<unsigned char *>(computed_signature_buffer.data()),
          input_buffer.data());
      if (memcmp(computed_signature_buffer.data(), read_signature_buffer.data(),
                 signature_size) != 0) {
        return false;
      }
    }
  }
  return true;
}

bool Delta::IsEmpty() {
  std::ifstream fin(m_in_file_name, std::ifstream::binary);
  if (!fin.eof()) {
    char val;
    if (!fin.read(&val, 1)) {
      return true;
    }
  }
  return false;
}

void Delta::CreateEmpty() {
  std::ofstream fout(m_out_delta_file_name, std::ofstream::binary);
  fout.write(&m_empty_delimiter, 1);
}

bool Delta::CompressionSM::AddBlock(uint32_t block_number,
                                    std::pair<uint32_t, uint32_t> &range) {
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

bool Delta::CompressionSM::Reset(std::pair<uint32_t, uint32_t> &range) {
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

void Delta::ParseDelta() {
  std::ifstream fin(m_out_delta_file_name, std::ifstream::binary);
  char delimiter;
  std::vector<char> data_buffer(m_block_size);

  while (!fin.eof()) {
    fin.read(&delimiter, 1);
    if (fin.gcount()) {
      if (delimiter == m_block_delimiter) {
        fin.read(data_buffer.data(), m_block_num_field_size);
        uint32_t block_number =
            *reinterpret_cast<uint32_t *>(data_buffer.data());
        printf("block_number: %d\n", block_number);
      } else if (delimiter == m_data_delimiter) {
        fin.read(data_buffer.data(), m_data_num_field_size);
        uint32_t data_length =
            *reinterpret_cast<uint32_t *>(data_buffer.data());
        printf("data_length: %d\n", data_length);

        uint32_t buffs_number = data_length / m_block_size;
        uint32_t buff_tail_size = data_length % m_block_size;
        while (buffs_number--) {
          fin.read(data_buffer.data(), m_block_size);
          for (const auto &a : data_buffer) {
            printf("val: %c, %x\n", a, a);
          }
        }
        fin.read(data_buffer.data(), buff_tail_size);
        for (uint32_t i = 0; i < buff_tail_size; ++i) {
          printf("val: %c, %x\n", data_buffer.at(i), data_buffer.at(i));
        }
      }
    }
  }
}
