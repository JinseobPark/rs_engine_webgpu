#include "SeobJJangApp.h"
#include "engine/core/math/Vec3.h"
#include "engine/systems/rendering/RenderSystem.h"
#include "engine/systems/resource/ResourceSystem.h"
#include "engine/systems/physics/PhysicsSystem.h"
#include "engine/rendering/scene/Scene.h"

using rs_engine::Vec3;
using rs_engine::RenderSystem;
using rs_engine::ResourceSystem;
using rs_engine::PhysicsSystem;

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

    // Cache system references
    renderSystem = engine.getSystem<RenderSystem>();
    resourceSystem = engine.getSystem<ResourceSystem>();
    
    if (!renderSystem || !resourceSystem ) {
        std::cerr << "[ERROR] Required systems not found!" << std::endl;
        return false;
    }

    // Start engine
    engine.start();

    // Setup scene using direct system access
    setupScene();
    std::cout << "Scene setup complete\n" << std::endl;
    return true;
}

void SeobJJangApp::setupScene() {
    // Direct system access - cleaner, more explicit code!
    
    auto* scene = renderSystem->getScene();
    if (!scene) {
        std::cerr << "[ERROR] Scene not available" << std::endl;
        return;
    }
    
    // Create mesh resources
    uint64_t cubeMeshHandle = resourceSystem->createCubeMesh("CubeMesh", 1.0f);
    uint64_t planeMeshHandle = resourceSystem->createPlaneMesh("PlaneMesh", 10.0f, 10.0f);
    
    // Create scene objects and set properties directly
    auto* cube1 = scene->createObject("Cube1");
    auto* cube2 = scene->createObject("Cube2");
    auto* cube3 = scene->createObject("Cube3");
    auto* plane1 = scene->createObject("Plane1");
    
    if (cube1 && cube2 && cube3 && plane1) {
        // Add meshes
        scene->addMeshToObject("Cube1", cubeMeshHandle);
        scene->addMeshToObject("Cube2", cubeMeshHandle);
        scene->addMeshToObject("Cube3", cubeMeshHandle);
        scene->addMeshToObject("Plane1", planeMeshHandle);
        
        // Set positions directly
        cube1->setPosition(Vec3(-2.0f, 0.0f, 0.0f));
        cube2->setPosition(Vec3(0.0f, 0.0f, 0.0f));
        cube3->setPosition(Vec3(2.0f, 0.0f, 0.0f));
        plane1->setPosition(Vec3(0.0f, 0.0f, 0.0f));
        
        std::cout << "   [INFO] Created 3 scene objects with cube meshes" << std::endl;
    }
    
    // Setup camera directly
    auto* camera = scene->getCamera();
    if (camera) {
        camera->setPosition(Vec3(0.0f, 2.0f, 5.0f));
        camera->setTarget(Vec3(0.0f, 0.0f, 0.0f));
        camera->setFOV(60.0f);
        std::cout << "   [INFO] Camera positioned at (0, 2, 5)" << std::endl;
    }
    
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
    // Native: Direct loop
    std::cout << "[INFO] Starting main loop...\n" << std::endl;
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
