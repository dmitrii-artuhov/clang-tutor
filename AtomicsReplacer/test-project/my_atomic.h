#pragma once

#include <iostream>

#include "other_atomic.h"

using namespace custom;

template<class T>
struct MyAtomic {
    OtherAtomic<T> a;

    T get() {
        std::cout << "MyAtomic get()" << std::endl;
        return a.get();
    }

    T incrementAndGet() {
        std::cout << "MyAtomic incrementAndGet()" << std::endl;
        return a.incrementAndGet();
    }
};