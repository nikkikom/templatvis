#include <iostream>
#include "throwers/throws.hpp"
#include "demo/vec.hpp"
#include "common/base.hpp"

template <typename T>
static const char* is(common::Base* p) { return dynamic_cast<T*>(p) ? "yes" : "no"; }

int main() {
    auto pi = throws::make_int();
    auto pd = throws::make_double();

    std::cout << "Vec<int> : "  << is<demo::Vec<int>>(pi)    << "\n";
    std::cout << "Vec<double>: " << is<demo::Vec<double>>(pd) << "\n";

    delete pi;
    delete pd;
    return 0;
}
