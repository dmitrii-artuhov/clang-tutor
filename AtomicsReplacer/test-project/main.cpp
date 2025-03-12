// #include <iostream>

#include "other_atomic.h"
#include "my_atomic.h"

int main() {
    auto a1 = custom::OtherAtomic<char>{};
    custom::OtherAtomic<char> a2;
    std::cout << "Atomic incremented: " << a1.incrementAndGet() << std::endl;
}

using namespace custom;

void fun(const custom::OtherAtomic<float>& other) {
    OtherAtomic  <long> a3{};
    OtherAtomic<int> a4 = OtherAtomic<int>{};
    OtherAtomic<int> a5 = OtherAtomic<int>();
    OtherAtomic<long>* ptr;
}