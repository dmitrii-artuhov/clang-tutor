#pragma once

#include <iostream>
#include <atomic>

template<class T>
struct MyAtomic {
    std::atomic<T> a;

    T load(std::memory_order order = std::memory_order_seq_cst) const {
        std::cout << "MyAtomic load()" << std::endl;
        return a.load(order);
    }

    void store(T desired, std::memory_order order = std::memory_order_seq_cst) noexcept {
        std::cout << "MyAtomic store()" << std::endl;
        a.store(desired, order);
    }

    T fetch_add(T arg, std::memory_order order = std::memory_order_seq_cst) noexcept {
        std::cout << "MyAtomic fetch_add()" << std::endl;
        return a.fetch_add(arg, order);
    }
};