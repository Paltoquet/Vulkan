#include "core/Application.h"
#include <core/Platform.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib> 


int main() {
    //Application app;
    Platform platform;

    try {
        platform.initialize();
        platform.mainLoop();
        platform.cleanUp();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}