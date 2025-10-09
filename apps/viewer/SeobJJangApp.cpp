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
#include <emscripten/bind.h>
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

#ifdef __EMSCRIPTEN__
// JavaScript API for HTML GUI integration
using namespace emscripten;

// Global app instance pointer for JS access (defined here)
SeobJJangApp* g_appInstance = nullptr;

// Camera data structure for JavaScript
struct CameraInfo {
    float posX, posY, posZ;
    float targetX, targetY, targetZ;
    float upX, upY, upZ;
    float fov;
    float nearPlane;
    float farPlane;
    float aspectRatio;
};

// Object data structure for JavaScript
struct ObjectInfo {
    std::string name;
    float posX, posY, posZ;
    float rotX, rotY, rotZ;
    float scaleX, scaleY, scaleZ;
    bool visible;
    bool hasModel;
};

// API functions
CameraInfo getCameraInfo() {
    CameraInfo info = {0, 0, 0, 0, 0, 0, 0, 1, 0, 60, 0.1f, 100.0f, 1.0f};

    if (g_appInstance && g_appInstance->renderSystem) {
        auto* camera = g_appInstance->renderSystem->getCamera();
        if (camera) {
            auto pos = camera->getPosition();
            auto target = camera->getTarget();
            auto up = camera->getUp();

            info.posX = pos.x; info.posY = pos.y; info.posZ = pos.z;
            info.targetX = target.x; info.targetY = target.y; info.targetZ = target.z;
            info.upX = up.x; info.upY = up.y; info.upZ = up.z;
            info.fov = camera->getFOV();
            info.nearPlane = camera->getNearPlane();
            info.farPlane = camera->getFarPlane();
            info.aspectRatio = camera->getAspectRatio();
        }
    }

    return info;
}

int getObjectCount() {
    if (g_appInstance && g_appInstance->renderSystem) {
        auto* scene = g_appInstance->renderSystem->getScene();
        if (scene) {
            return static_cast<int>(scene->getObjectCount());
        }
    }
    return 0;
}

std::vector<std::string> getObjectNames() {
    std::vector<std::string> names;

    if (g_appInstance && g_appInstance->renderSystem) {
        auto* scene = g_appInstance->renderSystem->getScene();
        if (scene) {
            const auto& objects = scene->getAllObjects();
            for (const auto& [name, obj] : objects) {
                names.push_back(name);
            }
        }
    }

    return names;
}

ObjectInfo getObjectInfo(const std::string& name) {
    ObjectInfo info;
    info.name = name;
    info.posX = info.posY = info.posZ = 0.0f;
    info.rotX = info.rotY = info.rotZ = 0.0f;
    info.scaleX = info.scaleY = info.scaleZ = 1.0f;
    info.visible = false;
    info.hasModel = false;

    if (g_appInstance && g_appInstance->renderSystem) {
        auto* scene = g_appInstance->renderSystem->getScene();
        if (scene) {
            auto* obj = scene->getObject(name);
            if (obj) {
                const auto& transform = obj->getTransform();
                info.posX = transform.position.x;
                info.posY = transform.position.y;
                info.posZ = transform.position.z;
                info.rotX = transform.rotation.x;
                info.rotY = transform.rotation.y;
                info.rotZ = transform.rotation.z;
                info.scaleX = transform.scale.x;
                info.scaleY = transform.scale.y;
                info.scaleZ = transform.scale.z;
                info.visible = obj->getVisible();
                info.hasModel = obj->hasModel();
            }
        }
    }

    return info;
}

std::string getSelectedObjectName() {
    if (g_appInstance && g_appInstance->renderSystem) {
        auto* obj = g_appInstance->renderSystem->getSelectedObject();
        if (obj) {
            std::string name = obj->getName();

            // Debug log (throttled)
            static int callCount = 0;
            callCount++;
            if (callCount % 60 == 0) { // Log every 60 calls (~1 second at 60fps)
                std::cout << "[Engine Debug] getSelectedObjectName() called: \"" << name << "\"" << std::endl;
            }

            return name;
        }
    }
    return "";
}

void selectObjectByName(const std::string& name) {
    if (!g_appInstance || !g_appInstance->renderSystem) {
        return;
    }

    auto* scene = g_appInstance->renderSystem->getScene();
    if (!scene) {
        return;
    }

    if (name.empty()) {
        // Clear selection
        scene->clearSelection();
    } else {
        // Select object by name
        auto* obj = scene->getObject(name);
        if (obj) {
            scene->setSelectedObject(obj);
            std::cout << "[GUI->Engine] Selected object: " << name << std::endl;
        } else {
            std::cout << "[GUI->Engine] Object not found: " << name << std::endl;
        }
    }
}

// Emscripten bindings
EMSCRIPTEN_BINDINGS(rs_engine_api) {
    // Register structures
    value_object<CameraInfo>("CameraInfo")
        .field("posX", &CameraInfo::posX)
        .field("posY", &CameraInfo::posY)
        .field("posZ", &CameraInfo::posZ)
        .field("targetX", &CameraInfo::targetX)
        .field("targetY", &CameraInfo::targetY)
        .field("targetZ", &CameraInfo::targetZ)
        .field("upX", &CameraInfo::upX)
        .field("upY", &CameraInfo::upY)
        .field("upZ", &CameraInfo::upZ)
        .field("fov", &CameraInfo::fov)
        .field("nearPlane", &CameraInfo::nearPlane)
        .field("farPlane", &CameraInfo::farPlane)
        .field("aspectRatio", &CameraInfo::aspectRatio)
        ;

    value_object<ObjectInfo>("ObjectInfo")
        .field("name", &ObjectInfo::name)
        .field("posX", &ObjectInfo::posX)
        .field("posY", &ObjectInfo::posY)
        .field("posZ", &ObjectInfo::posZ)
        .field("rotX", &ObjectInfo::rotX)
        .field("rotY", &ObjectInfo::rotY)
        .field("rotZ", &ObjectInfo::rotZ)
        .field("scaleX", &ObjectInfo::scaleX)
        .field("scaleY", &ObjectInfo::scaleY)
        .field("scaleZ", &ObjectInfo::scaleZ)
        .field("visible", &ObjectInfo::visible)
        .field("hasModel", &ObjectInfo::hasModel)
        ;

    // Register vector<string> for object names
    register_vector<std::string>("VectorString");

    // Register API functions
    function("getCameraInfo", &getCameraInfo);
    function("getObjectCount", &getObjectCount);
    function("getObjectNames", &getObjectNames);
    function("getObjectInfo", &getObjectInfo);
    function("getSelectedObjectName", &getSelectedObjectName);
    function("selectObjectByName", &selectObjectByName);
}
#endif
