
#include "Delta.h"
#include "Mdfour.h"
#include "Patch.h"
#include "Rabinkarp.h"
#include "Rollsum.h"
#include "Signature.h"
#include <array>
#include <fstream>
#include <iostream>

bool are_files_identical(const std::string &file_name_1,
                         const std::string &file_name_2) {
  std::ifstream fin_1(file_name_1, std::ifstream::binary);
  std::ifstream fin_2(file_name_2, std::ifstream::binary);

  char data_1, data_2;

  while (!fin_1.eof() && !fin_2.eof()) {
    fin_1.read(&data_1, 1);
    auto cnt_1 = fin_1.gcount();

    fin_2.read(&data_2, 1);
    auto cnt_2 = fin_2.gcount();

    // printf("data: %x, %x, cnt: %d, %d\n", data_1, data_2, cnt_1, cnt_2);

    if (cnt_1 != cnt_2) {
      return false;
    } else if ((cnt_1 == 1) && (cnt_2 == 1)) {
      if (data_1 != data_2) {
        return false;
      }
    }
  }
  return true;
}

int main() {
  std::cout << std::endl
            << "/*****************************hashing "
               "tests********************************/"
            << std::endl;
  {
    std::cout << "Rotation rollsum test" << std::endl;
    Rollsum r;
    std::array<unsigned char, 13> buf = {1, 2, 3, 4,  5,  16, 7,
                                         8, 9, 0, 11, 12, 13};
    r.Update(buf.data(), buf.size() - 1);
    uint32_t hash_0 = r.Digest();
    size_t count_0 = r.GetCount();

    r.Rotate(1, 13);
    uint32_t hash_1 = r.Digest();
    size_t count_1 = r.GetCount();
    std::cout << "hash_0: " << std::hex << hash_0 << " != hash_1: " << std::hex
              << hash_1 << " result: " << (hash_0 != hash_1) << std::endl;
    std::cout << "count_0: " << std::dec << count_0
              << " == count_1: " << count_1
              << " result: " << (count_0 == count_1) << std::endl;

    r.Reset();
    r.Update(buf.data() + 1, buf.size() - 1);
    uint32_t hash_2 = r.Digest();
    size_t count_2 = r.GetCount();
    std::cout << "hash_1: " << std::hex << hash_1 << " == hash_2: " << std::hex
              << hash_2 << " result: " << (hash_1 == hash_2) << std::endl;
    std::cout << "count_1: " << std::dec << count_1
              << " == count_2: " << count_2
              << " result: " << (count_1 == count_2) << std::endl;
  }

  {
    std::cout << "Rollin rollsum test" << std::endl;
    Rollsum r;
    std::array<unsigned char, 13> buf = {1, 2, 3, 4,  5,  16, 7,
                                         8, 9, 0, 11, 12, 13};
    r.Update(buf.data(), buf.size() - 1);
    uint32_t hash_0 = r.Digest();
    size_t count_0 = r.GetCount();

    r.Rollin(13);
    uint32_t hash_1 = r.Digest();
    size_t count_1 = r.GetCount();
    std::cout << "hash_0: " << std::hex << hash_0 << " != hash_1: " << std::hex
              << hash_1 << " result: " << (hash_0 != hash_1) << std::endl;
    std::cout << "count_0 + 1: " << std::dec << count_0 + 1
              << " == count_1: " << count_1
              << " result: " << ((count_0 + 1) == count_1) << std::endl;

    r.Reset();
    r.Update(buf.data(), buf.size());
    uint32_t hash_2 = r.Digest();
    size_t count_2 = r.GetCount();
    std::cout << "hash_1: " << std::hex << hash_1 << " == hash_2: " << std::hex
              << hash_2 << " result: " << (hash_1 == hash_2) << std::endl;
    std::cout << "count_1: " << std::dec << count_1
              << " == count_2: " << count_2
              << " result: " << (count_1 == count_2) << std::endl;
  }

  {
    std::cout << "Rollout rollsum test" << std::endl;
    Rollsum r;
    std::array<unsigned char, 13> buf = {1, 2, 3, 4,  5,  16, 7,
                                         8, 9, 0, 11, 12, 13};
    r.Update(buf.data(), buf.size());
    uint32_t hash_0 = r.Digest();
    size_t count_0 = r.GetCount();

    r.Rollout(1);
    uint32_t hash_1 = r.Digest();
    size_t count_1 = r.GetCount();
    std::cout << "hash_0: " << std::hex << hash_0 << " != hash_1: " << std::hex
              << hash_1 << " result: " << (hash_0 != hash_1) << std::endl;
    std::cout << "count_0: " << std::dec << count_0
              << " == count_1 + 1: " << count_1 + 1
              << " result: " << (count_0 == (count_1 + 1)) << std::endl;

    r.Reset();
    r.Update(buf.data() + 1, buf.size() - 1);
    uint32_t hash_2 = r.Digest();
    size_t count_2 = r.GetCount();
    std::cout << "hash_1: " << std::hex << hash_1 << " == hash_2: " << std::hex
              << hash_2 << " result: " << (hash_1 == hash_2) << std::endl;
    std::cout << "count_1: " << std::dec << count_1
              << " == count_2: " << count_2
              << " result: " << (count_1 == count_2) << std::endl;
  }

  {
    std::cout << "Rotation rabinkarp test" << std::endl;
    Rabinkarp r;
    std::array<unsigned char, 13> buf = {1, 2, 3, 4,  5,  16, 7,
                                         8, 9, 0, 11, 12, 13};
    r.Update(buf.data(), buf.size() - 1);
    uint32_t hash_0 = r.Digest();
    size_t count_0 = r.GetCount();

    r.Rotate(1, 13);
    uint32_t hash_1 = r.Digest();
    size_t count_1 = r.GetCount();
    std::cout << "hash_0: " << std::hex << hash_0 << " != hash_1: " << std::hex
              << hash_1 << " result: " << (hash_0 != hash_1) << std::endl;
    std::cout << "count_0: " << std::dec << count_0
              << " == count_1: " << count_1
              << " result: " << (count_0 == count_1) << std::endl;

    r.Reset();
    r.Update(buf.data() + 1, buf.size() - 1);
    uint32_t hash_2 = r.Digest();
    size_t count_2 = r.GetCount();
    std::cout << "hash_1: " << std::hex << hash_1 << " == hash_2: " << std::hex
              << hash_2 << " result: " << (hash_1 == hash_2) << std::endl;
    std::cout << "count_1: " << std::dec << count_1
              << " == count_2: " << count_2
              << " result: " << (count_1 == count_2) << std::endl;
  }

  {
    std::cout << "Rollin rabinkarp test" << std::endl;
    Rabinkarp r;
    std::array<unsigned char, 13> buf = {1, 2, 3, 4,  5,  16, 7,
                                         8, 9, 0, 11, 12, 13};
    r.Update(buf.data(), buf.size() - 1);
    uint32_t hash_0 = r.Digest();
    size_t count_0 = r.GetCount();

    r.Rollin(13);
    uint32_t hash_1 = r.Digest();
    size_t count_1 = r.GetCount();
    std::cout << "hash_0: " << std::hex << hash_0 << " != hash_1: " << std::hex
              << hash_1 << " result: " << (hash_0 != hash_1) << std::endl;
    std::cout << "count_0 + 1: " << std::dec << count_0 + 1
              << " == count_1: " << count_1
              << " result: " << ((count_0 + 1) == count_1) << std::endl;

    r.Reset();
    r.Update(buf.data(), buf.size());
    uint32_t hash_2 = r.Digest();
    size_t count_2 = r.GetCount();
    std::cout << "hash_1: " << std::hex << hash_1 << " == hash_2: " << std::hex
              << hash_2 << " result: " << (hash_1 == hash_2) << std::endl;
    std::cout << "count_1: " << std::dec << count_1
              << " == count_2: " << count_2
              << " result: " << (count_1 == count_2) << std::endl;
  }

  {
    std::cout << "Rollout rabinkarp test" << std::endl;
    Rabinkarp r;
    std::array<unsigned char, 13> buf = {1, 2, 3, 4,  5,  16, 7,
                                         8, 9, 0, 11, 12, 13};
    r.Update(buf.data(), buf.size());
    uint32_t hash_0 = r.Digest();
    size_t count_0 = r.GetCount();

    r.Rollout(1);
    uint32_t hash_1 = r.Digest();
    size_t count_1 = r.GetCount();
    std::cout << "hash_0: " << std::hex << hash_0 << " != hash_1: " << std::hex
              << hash_1 << " result: " << (hash_0 != hash_1) << std::endl;
    std::cout << "count_0: " << std::dec << count_0
              << " == count_1 + 1: " << count_1 + 1
              << " result: " << (count_0 == (count_1 + 1)) << std::endl;

    r.Reset();
    r.Update(buf.data() + 1, buf.size() - 1);
    uint32_t hash_2 = r.Digest();
    size_t count_2 = r.GetCount();
    std::cout << "hash_1: " << std::hex << hash_1 << " == hash_2: " << std::hex
              << hash_2 << " result: " << (hash_1 == hash_2) << std::endl;
    std::cout << "count_1: " << std::dec << count_1
              << " == count_2: " << count_2
              << " result: " << (count_1 == count_2) << std::endl;
  }

  {
    std::cout << "Mdfour test" << std::endl;
    Mdfour m;
    std::array<unsigned char, 4> buf = {'t', 'e', 's', 't'};
    std::array<unsigned char, 16> ref = {0xdb, 0x34, 0x6d, 0x69, 0x1d, 0x7a,
                                         0xcc, 0x4d, 0xc2, 0x62, 0x5d, 0xb1,
                                         0x9f, 0x9e, 0x3f, 0x52};

    std::array<unsigned char, 16> md4;
    m.Digest(md4.data(), buf.data(), buf.size());
    bool res = true;
    int i = 0;
    for (const auto &w : md4) {
      std::cout << std::hex << (int)w;
      if (w != ref.at(i)) {
        res = false;
      }
      ++i;
    }
    std::cout << "\nis md4 correct: " << res << std::endl;

    std::array<unsigned char, 4> buf2 = {'r', 'e', 's', 't'};
    std::array<unsigned char, 16> ref2 = {0xb8, 0xa2, 0x91, 0x7b, 0x60, 0xb9,
                                          0xdc, 0x1f, 0x16, 0x2c, 0xba, 0xdb,
                                          0x8d, 0xe0, 0xe3, 0xca};

    m.Digest(md4.data(), buf2.data(), buf2.size());
    res = true;
    i = 0;
    for (const auto &w : md4) {
      std::cout << std::hex << (int)w;
      if (w != ref2.at(i)) {
        res = false;
      }
      ++i;
    }
    std::cout << "\nis md4 correct: " << res << std::endl;
  }

  std::cout << std::endl
            << "/*****************************signature, delta, patching "
               "tests********************************/"
            << std::endl;
  const int tests_begin = 0;
  const int tests_end = 8;
  std::string dir;
  for (int i = tests_begin; i <= tests_end; ++i) {
    dir = std::to_string(i) + "/";

    size_t block_size = 32;
    Rollsum ws;
    Mdfour ss;

    const std::string in_file_name = dir + "input.txt";
    const std::string in_sig_file_name = dir + "sig.bin";
    {
      Signature s(ws, ss, in_file_name, in_sig_file_name, block_size);
      s.CreateSignature();
    }

    const std::string in_file_changed_name = dir + "input_changed.txt";
    const std::string out_delta_file_name = dir + "delta.bin";
    {
      Delta d(ws, ss, in_file_changed_name, in_sig_file_name,
              out_delta_file_name, block_size);
      d.CreateDelta();
    }

    const std::string out_recovered_file_name = dir + "recovered.bin";
    {
      Patch p(in_file_name, out_delta_file_name, out_recovered_file_name,
              block_size);
      p.MakePatch();
    }
  }

  for (int i = tests_begin; i <= tests_end; ++i) {
    dir = std::to_string(i) + "/";
    const std::string in_file_changed_name = dir + "input_changed.txt";
    const std::string out_recovered_file_name = dir + "recovered.bin";

    std::cout << "test: " << i << " result: "
              << are_files_identical(in_file_changed_name,
                                     out_recovered_file_name)
              << std::endl;
  }

  return 1;
}
