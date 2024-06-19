#include <cassert>
#include <iostream>
#include "functions.h"

void testGetCurrentDir() {
    std::string dir = getCurrentDir();
    assert(!dir.empty());
    std::cout << "Current Directory: " << dir << std::endl;
}

int main() {
    testGetCurrentDir();
    return 0;
}
