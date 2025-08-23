#include <gtest/gtest.h>
#include "demo/vec.hpp"
#include "throwers/throws.hpp"
using namespace demo;

TEST(Visibility, DynamicCastWorks) {
  {
    auto* p = throws::make_int();
    EXPECT_NE(dynamic_cast<Vec<int>*>(p), nullptr);
    delete p;
  }
  {
    auto* p = throws::make_double();
    EXPECT_NE(dynamic_cast<Vec<double>*>(p), nullptr);
    delete p;
  }
}
