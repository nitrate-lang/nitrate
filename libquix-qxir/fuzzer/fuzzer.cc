#include <stdexcept>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  (void)Data;
  (void)Size;
  
  /// TODO: Implement the fuzzer.
  throw std::runtime_error("Fuzzing not implemented.");
}