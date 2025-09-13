#pragma once

#ifdef __EMSCRIPTEN__
    #include "engine/platform/WebApplication.h"
    using BaseApp = rs_engine::WebApplication;
#else
    #include "engine/platform/NativeApplication.h"
    using BaseApp = rs_engine::NativeApplication;
#endif

class TriangleApp : public BaseApp {
public:
    bool onInit() override {
        std::cout << "🎯 Triangle App initialized successfully!" << std::endl;
        std::cout << "🎮 Controls:" << std::endl;
        std::cout << "   - Press ESC to close" << std::endl;
        return true;
    }
    
    void update(float deltaTime) override {
        // 여기에 공통 업데이트 로직 추가
        BaseApp::update(deltaTime);
    }
    
    void draw() override {
        // 여기에 공통 렌더링 로직 추가
        BaseApp::draw();
    }
};
