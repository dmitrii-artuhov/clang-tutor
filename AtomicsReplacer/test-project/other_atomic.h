#pragma once

namespace custom {

template<class U>
struct OtherAtomic {
    U value = 0;

    U get() {
        return value;
    }

    U incrementAndGet() {
        return ++value;
    }
};

}