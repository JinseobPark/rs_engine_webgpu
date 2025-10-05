#include "FluidDemoApp.h"
#include <iostream>

int main() {
    FluidDemoApp app;
    
    if (!app.init()) {
        std::cerr << "[ERROR] Failed to initialize application" << std::endl;
        app.shutdown();
        return -1;
    }

    app.run();

#ifndef __EMSCRIPTEN__
    app.shutdown();
#endif

    std::cout << "[SUCCESS] Application started successfully." << std::endl;
    return 0;
}