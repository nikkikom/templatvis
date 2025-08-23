#pragma once
#include "common/export.hpp"

namespace common {
struct COMMON_API Base {
  virtual ~Base()
#if defined(COMMON_INLINE_DTOR)
    = default;   // inline
#else
    ;            // out-of-line
#endif
};
} // namespace common
