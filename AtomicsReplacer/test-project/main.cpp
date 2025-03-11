#include <iostream>

#include "other_atomic.h"
#include "my_atomic.h"

int main() {
    auto a1 = custom::OtherAtomic<int>{};
    custom::OtherAtomic<int> a2;
    // std::cout << "Atomic incremented: " << a.incrementAndGet() << std::endl;
}

using namespace custom;

void fun(const OtherAtomic<int>& other) {
    OtherAtomic  <long> a3{};
    // OtherAtomic<int> a4 = OtherAtomic<int>{};
    // OtherAtomic<int> a5 = OtherAtomic<int>();
}