#include <iostream>
#include <atomic>

#include "my_atomic.h"

struct DataStructure {
    std::atomic<int> other;

    int getVal() {
        return other.load();
    }
};

int main() {
    auto a1 = std::atomic<char>{};
    std::atomic<char> a2;
    auto prev = a1.load();
    a1.store(prev + 1);
    std::cout << "Atomic fetch_add: " << a1.fetch_add(1) << std::endl;
}

using namespace std;

void fun(const std::atomic<float*>& other) {
    atomic  <long> a3{};
    atomic<int> a4 = atomic<int>{};
    atomic<int> a5 = atomic<int>();
    atomic<long>* ptr;
}