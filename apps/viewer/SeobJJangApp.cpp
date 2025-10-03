#include "SeobJJangApp.h"
#include "engine/core/math/Vec3.h"

using rs_engine::Vec3;

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

SeobJJangApp::SeobJJangApp() {
    std::cout << "🎮 Creating SeobJJang App..." << std::endl;
}

SeobJJangApp::~SeobJJangApp() {
    shutdown();
}

bool SeobJJangApp::init() {
    // Initialize engine (systems added automatically)
    if (!engine.initialize()) {
        std::cerr << "❌ Failed to initialize engine" << std::endl;
        return false;
    }

    // Start engine
    engine.start();

    // Setup scene through Engine interface
    setupScene();
    std::cout << "Scene setup complete\n" << std::endl;
    return true;
}

void SeobJJangApp::setupScene() {
    // ✅ Use Engine interface ONLY - NO direct system access
    
    // Add scene objects
    engine.addSceneObject("Cube1", Vec3(-2.0f, 0.0f, 0.0f), Vec3(1.0f, 1.0f, 1.0f));
    engine.addSceneObject("Cube2", Vec3(0.0f, 0.0f, 0.0f), Vec3(1.0f, 1.0f, 1.0f));
    engine.addSceneObject("Cube3", Vec3(2.0f, 0.0f, 0.0f), Vec3(1.0f, 1.0f, 1.0f));
    
    // Setup camera through Engine ✅
    engine.setCameraPosition(Vec3(0.0f, 2.0f, 5.0f));
    engine.setCameraTarget(Vec3(0.0f, 0.0f, 0.0f));
    engine.setCameraFOV(60.0f);
    
    // Set physics quality through Engine ✅
    engine.setPhysicsQuality(1.0f);
    
    std::cout << "   📦 Added 3 cubes to scene" << std::endl;
    std::cout << "   📷 Camera positioned at (0, 2, 5)" << std::endl;
    std::cout << "   ⚙️  Physics quality set to 1.0" << std::endl;
}

void SeobJJangApp::run() {
#ifdef __EMSCRIPTEN__
    // Web: Set up main loop callback
    emscripten_set_main_loop_arg(
        [](void* userData) {
            SeobJJangApp* app = static_cast<SeobJJangApp*>(userData);
            app->engine.update();
        },
        this, 0, 1);
#else
    // Native: Direct loop - Use Engine interface ✅
    std::cout << "🔄 Starting main loop (using Engine::shouldClose())...\n" << std::endl;
    while (!engine.shouldClose()) {
        engine.update();
    }
    std::cout << "\n🛑 Main loop ended" << std::endl;
#endif
}

void SeobJJangApp::shutdown() {
    std::cout << "🧹 Cleaning up SeobJJang Viewer..." << std::endl;
    engine.shutdown();
    std::cout << "✅ Cleanup complete" << std::endl;
}
