#pragma once
#include "common/base.hpp"
#include "throwers/export.hpp"

namespace throws {
THROWERS_EXPORT common::Base* make_int();
THROWERS_EXPORT common::Base* make_double();
} // namespace throws
