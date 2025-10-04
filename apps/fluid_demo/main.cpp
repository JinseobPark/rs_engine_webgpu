#include "FluidDemoApp.h"
#include <iostream>

int main() {
    FluidDemoApp app;
    
    if (!app.init()) {
        std::cerr << "❌ Failed to initialize application" << std::endl;
        app.shutdown();
        return -1;
    }

    app.run();

#ifndef __EMSCRIPTEN__
    app.shutdown();
#endif

    std::cout << "✅ Application started successfully." << std::endl;
    return 0;
}