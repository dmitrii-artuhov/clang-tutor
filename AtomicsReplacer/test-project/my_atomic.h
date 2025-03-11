#pragma once

#include <iostream>

#include "other_atomic.h"

using namespace custom;

template<class U>
struct MyAtomic {
    OtherAtomic<U> a;

    U get() {
        std::cout << "MyAtomic get()" << std::endl;
        return a.get();
    }

    U incrementAndGet() {
        std::cout << "MyAtomic incrementAndGet()" << std::endl;
        return a.incrementAndGet();
    }
};