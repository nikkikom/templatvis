#include <gtest/gtest.h>
#include "demo/vec.hpp"
#include "throwers/throws.hpp"

TEST(Visibility, DynamicCastWorks) {
  {
    auto* p = throws::make_int();
    EXPECT_NE(dynamic_cast<demo::Vec<int>*>(p), nullptr);
    delete p;
  }
  {
    auto* p = throws::make_double();
    EXPECT_NE(dynamic_cast<demo::Vec<double>*>(p), nullptr);
    delete p;
  }
}
