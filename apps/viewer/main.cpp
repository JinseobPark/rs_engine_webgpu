#include "SeobJJangApp.h"

#ifdef __EMSCRIPTEN__
// External global for JS API access
extern SeobJJangApp* g_appInstance;
#endif

int main() {
    SeobJJangApp app;

#ifdef __EMSCRIPTEN__
    // Set global instance for JS API
    extern SeobJJangApp* g_appInstance;
    g_appInstance = &app;
#endif

    if (!app.init()) {
        std::cerr << "[ERROR] Failed to initialize application" << std::endl;
        return -1;
    }

    app.run();

#ifndef __EMSCRIPTEN__
    // Native: Explicit shutdown after main loop
    app.shutdown();
#endif

    std::cout << "[SUCCESS] Application completed successfully." << std::endl;
    return 0;
}
