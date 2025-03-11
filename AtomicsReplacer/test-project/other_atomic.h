#pragma once

namespace custom {

template<class T>
struct OtherAtomic {
    T value = 0;

    T get() {
        return value;
    }

    T incrementAndGet() {
        return ++value;
    }
};

}