#include <iostream>
#include <atomic>

#include "my_atomic.h"

struct DataStructure {
    MyAtomic<int> other;

    int getVal() {
        return other.load();
    }
};

int main() {
    auto a1 = MyAtomic<char>{};
    MyAtomic<char> a2;
    auto prev = a1.load();
    a1.store(prev + 1);
    std::cout << "Atomic fetch_add: " << a1.fetch_add(1) << std::endl;
}

using namespace std;

void fun(const MyAtomic<float *>& other) {
    MyAtomic<long> a3{};
    MyAtomic<int> a4 = MyAtomic<int>{};
    MyAtomic<int> a5 = MyAtomic<int>();
    MyAtomic<long>* ptr;
}