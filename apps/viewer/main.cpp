#include "SeobJJangApp.h"

int main() {
    SeobJJangApp app;

    if (!app.init()) {
        std::cerr << "❌ Failed to initialize application" << std::endl;
        return -1;
    }

    app.run();

#ifndef __EMSCRIPTEN__
    // Native: Explicit shutdown after main loop
    app.shutdown();
#endif

    std::cout << "✅ Application completed successfully." << std::endl;
    return 0;
}
