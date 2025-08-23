#include "common/base.hpp"
namespace common {
#if !defined(COMMON_INLINE_DTOR)
Base::~Base() = default;  // только когда out-of-line
#endif
} // namespace common
