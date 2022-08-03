
#include "Delta.h"
#include "Mdfour.h"
#include "Patch.h"
#include "Rabinkarp.h"
#include "Rollsum.h"
#include "Signature.h"
#include <array>
#include <fstream>
#include <iostream>

int main(int argc, char *argv[]) {
  size_t block_size = 32;
  if (argc < 2) {
    std::cerr << "too few arguments" << std::endl;
    std::cerr << "possible operations: signature, delta, patch" << std::endl;
    return 1;
  }

  if (0 == strcmp(argv[1], "signature")) {
    if (argc < 4) {
      std::cerr << "too few arguments" << std::endl;
      std::cerr << "expected arguments for signature: input_file_name, "
                   "output_signature_file_name"
                << std::endl;
      return 1;
    }

    Rollsum ws;
    Mdfour ss;

    Signature s(ws, ss, argv[2], argv[3], block_size);
    s.CreateSignature();
  } else if (0 == strcmp(argv[1], "delta")) {
    if (argc < 5) {
      std::cerr << "too few arguments" << std::endl;
      std::cerr << "expected arguments for delta: input_changed_file_name, "
                   "input_signature_file_name, output_delta_file_name"
                << std::endl;
      return 1;
    }

    Rollsum ws;
    Mdfour ss;

    Delta d(ws, ss, argv[2], argv[3], argv[4], block_size);
    d.CreateDelta();
  } else if (0 == strcmp(argv[1], "patch")) {
    if (argc < 5) {
      std::cerr << "too few arguments" << std::endl;
      std::cerr << "expected arguments for patch: input_file_name, "
                   "input_delta_file_name, output_patched_file_name"
                << std::endl;
      return 1;
    }

    Patch p(argv[2], argv[3], argv[4], block_size);
    p.MakePatch();
  } else {
    std::cerr << "incorrect operation argument" << std::endl;
    return 1;
  }

  return 0;
}
