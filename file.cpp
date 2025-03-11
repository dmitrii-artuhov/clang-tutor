#include <iostream>

#include "file.h"


void foo() {
    std::cout << "Func foo\n";
}

int main() {
    Wrapper w;
    w.foo();
    foo();
    std::cout << "Hi from main" << std::endl;
    return 0;
}