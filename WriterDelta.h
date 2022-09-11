#ifndef WRITERDELTA_H
#define WRITERDELTA_H

#include "FileIO.h"
#include <vector>

class WriterDelta {
public:
  virtual void WriteFile(uint32_t block_num) = 0;
  virtual void WriteData(char front) = 0;
  virtual void WriteTail() = 0;

private:
  std::vector<char> m_file_data_buffer;
  const char m_block_delimiter{'b'};
  const char m_compressed_delimiter{'c'};
  const int m_block_num_field_size{4};

  const char m_data_delimiter{'d'};
  const int m_data_num_field_size{4};

  const char m_empty_delimiter{'e'};

protected:
  void WriteFileImpl(FileOut &fout, uint32_t block_num);
  void WriteTailImpl(FileOut &fout);
  void WriteDataImpl(char data);
  void WriteCompressedFileImpl(FileOut &fout,
                               const std::pair<uint32_t, uint32_t> &range);
};

class WriterDeltaBasic : public WriterDelta {
public:
  WriterDeltaBasic(FileOut &fout) : m_fout(fout) {}

  void WriteFile(uint32_t block_num) override {
    WriteFileImpl(m_fout, block_num);
  }

  void WriteData(char front) override { WriteDataImpl(front); }

  void WriteTail() override { WriteTailImpl(m_fout); }

private:
  FileOut &m_fout;
};

class WriterDeltaCompressed : public WriterDelta {
public:
  WriterDeltaCompressed(FileOut &fout) : m_fout(fout) {}

  void WriteFile(uint32_t block_num) override {
    std::pair<uint32_t, uint32_t> range;
    if (m_csm.AddBlock(block_num, range)) {
      WriteCompressedFileImpl(m_fout, range);
    }
  }

  void WriteData(char front) override {
    std::pair<uint32_t, uint32_t> range;
    if (m_csm.Reset(range)) {
      WriteCompressedFileImpl(m_fout, range);
    }
    WriteDataImpl(front);
  }

  void WriteTail() override {
    std::pair<uint32_t, uint32_t> range;
    if (m_csm.Reset(range)) {
      WriteCompressedFileImpl(m_fout, range);
    }
    WriteTailImpl(m_fout);
  }

private:
  class CompressionSM {
  public:
    bool AddBlock(uint32_t block_number, std::pair<uint32_t, uint32_t> &range);
    bool Reset(std::pair<uint32_t, uint32_t> &range);

  private:
    uint32_t m_first, m_last;
    uint32_t m_state{0};
  };

  CompressionSM m_csm;
  FileOut &m_fout;
};

#endif
