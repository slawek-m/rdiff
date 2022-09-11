#include "Delta.h"
#include "WriterDelta.h"
#include <iostream>
#include <memory>

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

void Delta::CreateDelta(bool is_compressed_mode) {
  if (IsIdentical()) {
    FileOut fout(m_out_delta_file_name, std::ofstream::binary);
    return;
  }

  if (IsInputEmpty()) {
    CreateEmpty();
    return;
  }

  FileIn fin(m_in_file_name, std::ifstream::binary);
  FileOut fout(m_out_delta_file_name, std::ofstream::binary);

  std::unique_ptr<WriterDelta> writer;
  if (is_compressed_mode) {
    writer = std::make_unique<WriterDeltaCompressed>(fout);
  } else {
    writer = std::make_unique<WriterDeltaBasic>(fout);
  }

  char buff;
  char front;
  uint32_t weak_sig;
  uint32_t block_num;

  while (!fin.Eof()) {
    fin.Read(&buff, 1);
    if (fin.Count()) {
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

          writer->WriteFile(block_num);
        }
      } else if (size > m_block_size) {
        front = m_dq.front();
        m_dq.pop_front();
        m_ws.Rotate(front, buff);
        weak_sig = m_ws.Digest();

        writer->WriteData(front);

        if (Match(weak_sig, block_num)) {
          m_dq.clear();

          writer->WriteFile(block_num);
        }
      }
    }
  }

  while (m_dq.size()) {
    weak_sig = m_ws.Digest();
    if (Match(weak_sig, block_num)) {
      m_dq.clear();

      writer->WriteFile(block_num);
    } else {
      front = m_dq.front();
      m_dq.pop_front();
      m_ws.Rollout(front);

      writer->WriteData(front);
    }
  }
  writer->WriteTail();
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

bool Delta::IsIdentical() {
  FileIn fin(m_in_file_name, std::ifstream::binary);
  FileIn fsig(m_in_sig_file_name, std::ifstream::binary);

  std::vector<char> input_buffer(m_block_size);
  const size_t signature_size = m_ws.GetSize() + m_ss.GetSize();
  std::vector<char> computed_signature_buffer(signature_size);
  std::vector<char> read_signature_buffer(signature_size);

  while (!fin.Eof() && !fsig.Eof()) {
    fin.Read(input_buffer.data(), input_buffer.size());
    size_t actual_block_size = fin.Count();

    fsig.Read(read_signature_buffer.data(), read_signature_buffer.size());
    size_t actual_signature_size = fsig.Count();

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

bool Delta::IsInputEmpty() {
  FileIn fin(m_in_file_name, std::ifstream::binary);
  bool ret;
  fin.Length() ? ret = false : ret = true;
  return ret;
}

void Delta::CreateEmpty() {
  FileOut fout(m_out_delta_file_name, std::ofstream::binary);
  fout.Write(&m_empty_delimiter, 1);
}

void Delta::ParseDelta() {
  FileIn fin(m_out_delta_file_name, std::ifstream::binary);
  char delimiter;
  std::vector<char> data_buffer(m_block_size);

  while (!fin.Eof()) {
    fin.Read(&delimiter, 1);
    if (fin.Count()) {
      if (delimiter == m_block_delimiter) {
        fin.Read(data_buffer.data(), m_block_num_field_size);
        uint32_t block_number =
            *reinterpret_cast<uint32_t *>(data_buffer.data());
        printf("block_number: %d\n", block_number);
      } else if (delimiter == m_data_delimiter) {
        fin.Read(data_buffer.data(), m_data_num_field_size);
        uint32_t data_length =
            *reinterpret_cast<uint32_t *>(data_buffer.data());
        printf("data_length: %d\n", data_length);

        uint32_t buffs_number = data_length / m_block_size;
        uint32_t buff_tail_size = data_length % m_block_size;
        while (buffs_number--) {
          fin.Read(data_buffer.data(), m_block_size);
          for (const auto &a : data_buffer) {
            printf("val: %c, %x\n", a, a);
          }
        }
        fin.Read(data_buffer.data(), buff_tail_size);
        for (uint32_t i = 0; i < buff_tail_size; ++i) {
          printf("val: %c, %x\n", data_buffer.at(i), data_buffer.at(i));
        }
      }
    }
  }
}
