#ifndef RABINKARP_H
#define RABINKARP_H

#include "Weaksum.h"

/* Constant for RABINKARP_MULT^2. */
#define RABINKARP_MULT2 0xa5b71959U

/* Macros for doing 16 bytes with 2 mults that can be done in parallel. Testing
   showed this as a performance sweet spot vs 16x1, 8x2, 4x4 1x16 alternative
   arrangements. */
#define PAR2X1(hash, buf, i)                                                   \
  (RABINKARP_MULT2 * (hash) + RABINKARP_MULT * buf[i] + buf[i + 1])
#define PAR2X2(hash, buf, i) PAR2X1(PAR2X1(hash, buf, i), buf, i + 2)
#define PAR2X4(hash, buf, i) PAR2X2(PAR2X2(hash, buf, i), buf, i + 4)
#define PAR2X8(hash, buf) PAR2X4(PAR2X4(hash, buf, 0), buf, 8)

class Rabinkarp : public Weaksum {
public:
  struct Sum {
    uint32_t hash; /**< The accumulated hash value. */
    uint32_t mult; /**< The value of RABINKARP_MULT^count. */
  };

  uint32_t Digest() { return m_sum.hash; }

  void Reset() {
    m_sum.hash = RABINKARP_SEED;
    m_sum.mult = 1;
    m_count = 0;
  }

  void Update(const unsigned char *buf, size_t len) override {
    size_t n = len;
    uint32_t hash = m_sum.hash;

    while (n >= 16) {
      hash = PAR2X8(hash, buf);
      buf += 16;
      n -= 16;
    }
    while (n) {
      hash = RABINKARP_MULT * hash + *buf++;
      n--;
    }
    m_sum.hash = hash;
    m_count += len;
    m_sum.mult *= rabinkarp_pow((uint32_t)len);
  }

  void Rotate(unsigned char out, unsigned char in) override {
    m_sum.hash =
        m_sum.hash * RABINKARP_MULT + in - m_sum.mult * (out + RABINKARP_ADJ);
  }

  void Rollin(unsigned char in) override {
    m_sum.hash = m_sum.hash * RABINKARP_MULT + in;
    m_count++;
    m_sum.mult *= RABINKARP_MULT;
  }

  void Rollout(unsigned char out) override {
    m_count--;
    m_sum.mult *= RABINKARP_INVM;
    m_sum.hash -= m_sum.mult * (out + RABINKARP_ADJ);
  }

  void *GetSum() const override { return (void *)&m_sum; }

  size_t GetSize() const override { return sizeof(m_sum.hash); }

private:
  uint32_t rabinkarp_pow(uint32_t n) {
    const uint32_t *m = RABINKARP_MULT_POW2;
    uint32_t ans = 1;
    while (n) {
      if (n & 1) {
        ans *= *m;
      }
      m++;
      n >>= 1;
    }
    return ans;
  }

  const uint32_t RABINKARP_SEED = 1;
  const uint32_t RABINKARP_MULT = 0x08104225U;
  const uint32_t RABINKARP_INVM = 0x98f009adU;
  const uint32_t RABINKARP_ADJ = 0x08104224U;

  const uint32_t RABINKARP_MULT_POW2[32] = {
      0x08104225U, 0xa5b71959U, 0xf9c080f1U, 0x7c71e2e1U, 0x0bb409c1U,
      0x4dc72381U, 0xd17a8701U, 0x96260e01U, 0x55101c01U, 0x2d303801U,
      0x66a07001U, 0xfe40e001U, 0xc081c001U, 0x91038001U, 0x62070001U,
      0xc40e0001U, 0x881c0001U, 0x10380001U, 0x20700001U, 0x40e00001U,
      0x81c00001U, 0x03800001U, 0x07000001U, 0x0e000001U, 0x1c000001U,
      0x38000001U, 0x70000001U, 0xe0000001U, 0xc0000001U, 0x80000001U,
      0x00000001U, 0x00000001U};

  Sum m_sum{RABINKARP_SEED, 1};
};

#endif
