#ifdef __EMSCRIPTEN__
    #include "TriangleApp.h"
#else
    // 네이티브 버전은 임시로 기본 GLFW 창만 구현
    #include <GLFW/glfw3.h>
    #include <iostream>

    static void errorCallback(int error, const char* description) {
        std::cerr << "GLFW Error " << error << ": " << description << std::endl;
    }

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }
#endif

int main() {
#ifdef __EMSCRIPTEN__
    // 웹 버전은 새로운 통합 엔진 사용
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
#else
    // 네이티브 버전은 임시로 기본 GLFW 창 구현
    std::cout << "🖥️ Starting Native WebGPU Demo (Basic Version)" << std::endl;
    
    // Initialize GLFW
    glfwSetErrorCallback(errorCallback);
    
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Create window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    
    GLFWwindow* window = glfwCreateWindow(800, 600, "WebGPU Triangle - Native (Basic)", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwSetKeyCallback(window, keyCallback);

    std::cout << "🎯 Basic GLFW window created!" << std::endl;
    std::cout << "🎮 Controls:" << std::endl;
    std::cout << "   - Press ESC to close" << std::endl;
    std::cout << "💡 Note: Full WebGPU integration coming soon!" << std::endl;

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glfwWaitEvents(); // Save CPU
    }

    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
    
    std::cout << "✅ Application terminated successfully." << std::endl;
    return 0;
#endif
}
