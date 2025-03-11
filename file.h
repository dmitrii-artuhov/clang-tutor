#pragma once

#include <iostream>


struct Wrapper {
    void bar() {
        std::cout << "bar()!\n";
    }

    void foo() {
        std::cout << "foo()!\n";
    }
};

inline void func() {
    Wrapper w;

    w.foo(); // must change to bar
}