#pragma once

#ifdef __EMSCRIPTEN__
    #include "engine/platform/WebApplication.h"
    using BaseApp = rs_engine::WebApplication;
#else
    #include "engine/platform/NativeApplication.h"
    using BaseApp = rs_engine::NativeApplication;
#endif

class SeobJJangApp : public BaseApp {
public:
    SeobJJangApp();
    ~SeobJJangApp();

    bool onInit() override;
    
    void update(float deltaTime) override;
    
    void draw() override;
};
