#ifndef ROLLSUM_H
#define ROLLSUM_H

#include "Weaksum.h"
#include <utility>

#define DO1(buf, i)                                                            \
  {                                                                            \
    s1 += buf[i];                                                              \
    s2 += s1;                                                                  \
  }
#define DO2(buf, i)                                                            \
  DO1(buf, i);                                                                 \
  DO1(buf, i + 1);
#define DO4(buf, i)                                                            \
  DO2(buf, i);                                                                 \
  DO2(buf, i + 2);
#define DO8(buf, i)                                                            \
  DO4(buf, i);                                                                 \
  DO4(buf, i + 4);
#define DO16(buf)                                                              \
  DO8(buf, 0);                                                                 \
  DO8(buf, 8);

class Rollsum : public Weaksum {
public:
  uint32_t Digest() {
    return ((uint32_t)m_sum.second << 16) | ((uint32_t)m_sum.first & 0xffff);
  }

  void Reset() {
    m_count = 0;
    m_sum.first = 0;
    m_sum.second = 0;
  }

  void Update(const unsigned char *buf, size_t len) override {
    size_t n = len;
    uint16_t s1 = m_sum.first;
    uint16_t s2 = m_sum.second;

    while (n >= 16) {
      DO16(buf);
      buf += 16;
      n -= 16;
    }
    while (n != 0) {
      s1 += *buf++;
      s2 += s1;
      n--;
    }
    /* Increment s1 and s2 by the amounts added by the char offset. */
    s1 += len * ROLLSUM_CHAR_OFFSET;
    s2 += ((len * (len + 1)) / 2) * ROLLSUM_CHAR_OFFSET;
    m_count += len; /* Increment sum count. */
    m_sum.first = s1;
    m_sum.second = s2;
  }

  void Rotate(unsigned char out, unsigned char in) override {
    m_sum.first += in - out;
    m_sum.second += m_sum.first - m_count * (out + ROLLSUM_CHAR_OFFSET);
  }

  void Rollin(unsigned char in) override {
    m_sum.first += in + ROLLSUM_CHAR_OFFSET;
    m_sum.second += m_sum.first;
    m_count++;
  }

  void Rollout(unsigned char out) override {
    m_sum.first -= out + ROLLSUM_CHAR_OFFSET;
    m_sum.second -= m_count * (out + ROLLSUM_CHAR_OFFSET);
    m_count--;
  }

  void *GetSum() const override { return (void *)&m_sum; }

  size_t GetSize() const override {
    return sizeof(m_sum.first) + sizeof(m_sum.second);
  }

private:
  const size_t ROLLSUM_CHAR_OFFSET{31};
  std::pair<uint16_t, uint16_t> m_sum{0, 0};
};

#endif
