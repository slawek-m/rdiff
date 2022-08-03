#ifndef MDFOUR_H
#define MDFOUR_H

#include "Strongsum.h"
#include <cstdint>
#include <cstring>

#define F(X, Y, Z) (((X) & (Y)) | ((~(X)) & (Z)))
#define G(X, Y, Z) (((X) & (Y)) | ((X) & (Z)) | ((Y) & (Z)))
#define H(X, Y, Z) ((X) ^ (Y) ^ (Z))
#define lshift(x, s) (((x) << (s)) | ((x) >> (32 - (s))))

#define ROUND1(a, b, c, d, k, s) a = lshift(a + F(b, c, d) + X[k], s)
#define ROUND2(a, b, c, d, k, s)                                               \
  a = lshift(a + G(b, c, d) + X[k] + 0x5A827999U, s)
#define ROUND3(a, b, c, d, k, s)                                               \
  a = lshift(a + H(b, c, d) + X[k] + 0x6ED9EBA1U, s)

class Mdfour : public Strongsum {
public:
  void Digest(unsigned char *out, void const *in, size_t n) override {
    Begin();
    Update(in, n);
    Result(out);
  }

  size_t GetSize() const override { return 16; }

private:
  void Mdfour64(void const *p) {
    uint32_t AA, BB, CC, DD;
    uint32_t A, B, C, D;
    const uint32_t *X = (const uint32_t *)p;

    A = m_A;
    B = m_B;
    C = m_C;
    D = m_D;
    AA = A;
    BB = B;
    CC = C;
    DD = D;

    ROUND1(A, B, C, D, 0, 3);
    ROUND1(D, A, B, C, 1, 7);
    ROUND1(C, D, A, B, 2, 11);
    ROUND1(B, C, D, A, 3, 19);
    ROUND1(A, B, C, D, 4, 3);
    ROUND1(D, A, B, C, 5, 7);
    ROUND1(C, D, A, B, 6, 11);
    ROUND1(B, C, D, A, 7, 19);
    ROUND1(A, B, C, D, 8, 3);
    ROUND1(D, A, B, C, 9, 7);
    ROUND1(C, D, A, B, 10, 11);
    ROUND1(B, C, D, A, 11, 19);
    ROUND1(A, B, C, D, 12, 3);
    ROUND1(D, A, B, C, 13, 7);
    ROUND1(C, D, A, B, 14, 11);
    ROUND1(B, C, D, A, 15, 19);

    ROUND2(A, B, C, D, 0, 3);
    ROUND2(D, A, B, C, 4, 5);
    ROUND2(C, D, A, B, 8, 9);
    ROUND2(B, C, D, A, 12, 13);
    ROUND2(A, B, C, D, 1, 3);
    ROUND2(D, A, B, C, 5, 5);
    ROUND2(C, D, A, B, 9, 9);
    ROUND2(B, C, D, A, 13, 13);
    ROUND2(A, B, C, D, 2, 3);
    ROUND2(D, A, B, C, 6, 5);
    ROUND2(C, D, A, B, 10, 9);
    ROUND2(B, C, D, A, 14, 13);
    ROUND2(A, B, C, D, 3, 3);
    ROUND2(D, A, B, C, 7, 5);
    ROUND2(C, D, A, B, 11, 9);
    ROUND2(B, C, D, A, 15, 13);

    ROUND3(A, B, C, D, 0, 3);
    ROUND3(D, A, B, C, 8, 9);
    ROUND3(C, D, A, B, 4, 11);
    ROUND3(B, C, D, A, 12, 15);
    ROUND3(A, B, C, D, 2, 3);
    ROUND3(D, A, B, C, 10, 9);
    ROUND3(C, D, A, B, 6, 11);
    ROUND3(B, C, D, A, 14, 15);
    ROUND3(A, B, C, D, 1, 3);
    ROUND3(D, A, B, C, 9, 9);
    ROUND3(C, D, A, B, 5, 11);
    ROUND3(B, C, D, A, 13, 15);
    ROUND3(A, B, C, D, 3, 3);
    ROUND3(D, A, B, C, 11, 9);
    ROUND3(C, D, A, B, 7, 11);
    ROUND3(B, C, D, A, 15, 15);

    A += AA;
    B += BB;
    C += CC;
    D += DD;

    m_A = A;
    m_B = B;
    m_C = C;
    m_D = D;
  }

  void Copy4(unsigned char *out, uint32_t const x) {
    out[0] = (unsigned char)(x);
    out[1] = (unsigned char)(x >> 8);
    out[2] = (unsigned char)(x >> 16);
    out[3] = (unsigned char)(x >> 24);
  }

  void Copy8(unsigned char *out, uint64_t const x) {
    out[0] = (unsigned char)(x);
    out[1] = (unsigned char)(x >> 8);
    out[2] = (unsigned char)(x >> 16);
    out[3] = (unsigned char)(x >> 24);
    out[4] = (unsigned char)(x >> 32);
    out[5] = (unsigned char)(x >> 40);
    out[6] = (unsigned char)(x >> 48);
    out[7] = (unsigned char)(x >> 56);
  }

  void Block(void const *p) {
    if ((uintptr_t)p & 3) {
      uint32_t M[16];

      memcpy(M, p, 16 * sizeof(uint32_t));
      Mdfour64(M);
    } else {
      Mdfour64((const uint32_t *)p);
    }
  }

  void Begin() {
    m_A = 0x67452301U;
    m_B = 0xefcdab89U;
    m_C = 0x98badcfeU;
    m_D = 0x10325476U;

    m_tail_len = 0;
    m_totalN = 0;
    memset(m_tail, 0, 64);
  }

  void Tail() {
    uint64_t b;
    unsigned char buf[8];
    size_t pad_len;

    /* convert the totalN byte count into a bit count buffer */
    b = m_totalN << 3;
    Copy8(buf, b);

    /* calculate length and process the padding data */
    pad_len = (m_tail_len < 56) ? (56 - m_tail_len) : (120 - m_tail_len);
    Update(PADDING, pad_len);
    /* process the bit count */
    Update(buf, 8);
  }

  void Update(void const *in_void, size_t n) {
    unsigned char const *in = (unsigned char const *)in_void;

    /* increment totalN */
    m_totalN += n;

    /* If there's any leftover data in the tail buffer, then first we have to
    make it up to a whole block to process it. */
    if (m_tail_len) {
      size_t tail_gap = 64 - m_tail_len;
      if (tail_gap <= n) {
        memcpy(&m_tail[m_tail_len], in, tail_gap);
        Block(m_tail);
        in += tail_gap;
        n -= tail_gap;
        m_tail_len = 0;
      }
    }
    /* process complete blocks of input */
    while (n >= 64) {
      Block(in);
      in += 64;
      n -= 64;
    }
    /* Put remaining bytes onto tail */
    if (n) {
      memcpy(&m_tail[m_tail_len], in, n);
      m_tail_len += (int)n;
    }
  }

  void Result(unsigned char *out) {
    Tail();

    Copy4(out, m_A);
    Copy4(out + 4, m_B);
    Copy4(out + 8, m_C);
    Copy4(out + 12, m_D);
  }

  /** padding data used for finalising */
  const unsigned char PADDING[64] = {
      0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  unsigned int m_A, m_B, m_C, m_D;
  uint64_t m_totalN;
  int m_tail_len;
  unsigned char m_tail[64];
};

#endif
