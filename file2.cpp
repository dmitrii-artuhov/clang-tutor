#include <iostream>

#include "file.h"


void file2Func() {
    std::cout << "Func func\n";
    Wrapper w;
    w.foo(); // must change from foo to bar
}