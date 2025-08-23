#pragma once
#include "throwers/export.hpp"
#include "common/base.hpp"

namespace throws {
THROWERS_EXPORT common::Base* make_int();
THROWERS_EXPORT common::Base* make_double();
} // namespace throws
