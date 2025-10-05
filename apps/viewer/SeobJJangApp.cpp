#include "SeobJJangApp.h"
#include "engine/core/math/Vec3.h"

using rs_engine::Vec3;

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

SeobJJangApp::SeobJJangApp() {
    std::cout << "[INFO] Creating SeobJJang App..." << std::endl;
}

SeobJJangApp::~SeobJJangApp() {
    shutdown();
}

bool SeobJJangApp::init() {
    // Initialize engine (systems added automatically)
    if (!engine.initialize()) {
        std::cerr << "[ERROR] Failed to initialize engine" << std::endl;
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
    // [OK] Use Engine interface ONLY - NO direct system access
    
    // Create cube mesh resource (shared by all objects)
    uint64_t cubeMeshHandle = engine.createCubeMesh("CubeMesh", 1.0f);
    uint64_t planeMeshHandle = engine.createPlaneMesh("PlaneMesh", 10.0f, 10.0f);
    
    // Create empty scene objects
    engine.createSceneObject("Cube1");
    engine.createSceneObject("Cube2");
    engine.createSceneObject("Cube3");
    engine.createSceneObject("Plane1");

    // Add cube mesh to each object
    engine.addMeshToSceneObject("Cube1", cubeMeshHandle);
    engine.addMeshToSceneObject("Cube2", cubeMeshHandle);
    engine.addMeshToSceneObject("Cube3", cubeMeshHandle);
    engine.addMeshToSceneObject("Plane1", planeMeshHandle);

    // Set positions
    engine.setObjectPosition("Cube1", Vec3(-2.0f, 0.0f, 0.0f));
    engine.setObjectPosition("Cube2", Vec3(0.0f, 0.0f, 0.0f));
    engine.setObjectPosition("Cube3", Vec3(2.0f, 0.0f, 0.0f));
    engine.setObjectPosition("Plane1", Vec3(0.0f, 0.0f, 0.0f));
    
    // Setup camera through Engine [OK]
    engine.setCameraPosition(Vec3(0.0f, 2.0f, 5.0f));
    engine.setCameraTarget(Vec3(0.0f, 0.0f, 0.0f));
    engine.setCameraFOV(60.0f);
    
    // Set physics quality through Engine [OK]
    engine.setPhysicsQuality(1.0f);
    
    std::cout << "   [INFO] Created 3 scene objects with cube meshes" << std::endl;
    std::cout << "   [INFO] Camera positioned at (0, 2, 5)" << std::endl;
    std::cout << "   [INFO] Physics quality set to 1.0" << std::endl;
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
    // Native: Direct loop - Use Engine interface [OK]
    std::cout << "[INFO] Starting main loop (using Engine::shouldClose())...\n" << std::endl;
    while (!engine.shouldClose()) {
        engine.update();
    }
    std::cout << "\n[INFO] Main loop ended" << std::endl;
#endif
}

void SeobJJangApp::shutdown() {
    std::cout << "[INFO] Cleaning up SeobJJang Viewer..." << std::endl;
    engine.shutdown();
    std::cout << "[SUCCESS] Cleanup complete" << std::endl;
}
