#pragma once

#include "engine/core/Engine.h"
#include <iostream>

/**
 * @brief SeobJJang Viewer Application
 * 
 * Demonstrates PERFECT encapsulation following Architecture Philosophy:
 * - [OK] App ONLY knows Engine class
 * - [OK] NO system types or headers
 * - [OK] NO manual system addition
 * - [OK] Engine handles all system management
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
    rs_engine::Engine engine;  // [OK] Only dependency

public:
    SeobJJangApp();
    ~SeobJJangApp();

    bool init();
    void run();
    void shutdown();

private:
    void setupScene();
};
