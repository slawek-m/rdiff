#ifndef STRONGSUM_H
#define STRONGSUM_H

#include <cstring>

class Strongsum {
public:
  virtual void Digest(unsigned char *out, void const *in, size_t n) = 0;
  virtual size_t GetSize() const = 0;
};

#endif
