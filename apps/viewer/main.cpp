#ifdef __EMSCRIPTEN__
    #include "TriangleApp.h"
#else
    #include "TriangleApp.h"
#endif

int main() {
#ifdef __EMSCRIPTEN__
    // 웹 버전: 비동기 실행으로 cleanup은 자동 처리
    TriangleApp app;
    
    if (!app.init()) {
        std::cerr << "❌ Failed to initialize application" << std::endl;
        app.cleanup();
        return -1;
    }
    
    app.run();
    // 웹에서는 cleanup이 emscripten 콜백에서 자동으로 처리됨
    
    std::cout << "✅ Application started successfully." << std::endl;
    return 0;
#else
    // 네이티브 버전: 동기 실행으로 명시적 cleanup 필요
    TriangleApp app;
    
    if (!app.init()) {
        std::cerr << "❌ Failed to initialize application" << std::endl;
        app.cleanup();
        return -1;
    }
    
    app.run();
    app.cleanup();
    
    std::cout << "✅ Application terminated successfully." << std::endl;
    return 0;
#endif
}
