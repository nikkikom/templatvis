#include "throwers/throws.hpp"
#include "demo/vec.hpp"

namespace throws {
common::Base* make_int()    { return new demo::Vec<int>(42); }
common::Base* make_double() { return new demo::Vec<double>(3.14); }
} // namespace throws
