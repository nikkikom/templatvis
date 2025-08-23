#include <cstddef>
#include <cstdint>

#include "demo/vec.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  if (size < sizeof(int))
    return 0;
  int v = 0;
  for (size_t i = 0; i < sizeof(int) && i < size; ++i)
    v = (v << 8) ^ data[i];
  demo::Vec<int> x(v);
  (void)x.get();
  return 0;
}
