#ifndef WEAKSUM_H
#define WEAKSUM_H

#include <cstddef>
#include <cstdint>

class Weaksum {
public:
  virtual uint32_t Digest() = 0;
  virtual void Reset() = 0;
  virtual void Update(const unsigned char *buf, size_t len) = 0;
  virtual void Rotate(unsigned char out, unsigned char in) = 0;
  virtual void Rollin(unsigned char in) = 0;
  virtual void Rollout(unsigned char out) = 0;
  virtual void *GetSum() const = 0;
  virtual size_t GetSize() const = 0;

  size_t GetCount() const { return m_count; };

protected:
  size_t m_count{0}; /**< count of bytes included in sum */
};

#endif
