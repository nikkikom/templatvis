#define DEMO_DO_INSTANTIATE 1
#include "demo/vec.hpp"

namespace demo {
#if !DEMO_INLINE_DTOR
template<> Vec<int>::~Vec() = default;
template<> Vec<double>::~Vec() = default;
#endif
} // namespace demo
