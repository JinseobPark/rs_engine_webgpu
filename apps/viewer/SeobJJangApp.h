#pragma once

#include "engine/core/Engine.h"
#include <iostream>

/**
 * @brief SeobJJang Viewer Application
 * 
 * Demonstrates PERFECT encapsulation following Architecture Philosophy:
 * - ✅ App ONLY knows Engine class
 * - ✅ NO system types or headers
 * - ✅ NO manual system addition
 * - ✅ Engine handles all system management
 * 
 * This app shows:
 * - Automatic system initialization by Engine
 * - Scene setup through Engine interface
 * - Camera control through Engine interface
 * - Physics control through Engine interface
 * - Main loop using Engine::shouldClose()
 */
class SeobJJangApp {
private:
    rs_engine::Engine engine;  // ✅ Only dependency

public:
    SeobJJangApp();
    ~SeobJJangApp();

    bool init();
    void run();
    void shutdown();

private:
    void setupScene();
};
