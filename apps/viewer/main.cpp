#ifdef __EMSCRIPTEN__
    #include "SeobJJangApp.h"
#else
    #include "SeobJJangApp.h"
#endif

int main() {
    // Unified initialization flow for both platforms
    SeobJJangApp app;

    if (!app.init()) {
        std::cerr << "❌ Failed to initialize application" << std::endl;
        app.cleanup();
        return -1;
    }

    app.run();

#ifndef __EMSCRIPTEN__
    // Only call cleanup explicitly on native platforms
    // Web platforms handle cleanup through emscripten callbacks
    app.cleanup();
#endif

    std::cout << "✅ Application started successfully." << std::endl;
    return 0;
}
