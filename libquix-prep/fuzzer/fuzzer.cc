#include <quix-prep/Lib.h>

#include <stdexcept>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  (void)Data;
  (void)Size;
  throw std::runtime_error("Not implemented");
  return 0;
}