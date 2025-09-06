#include <GLFW/glfw3.h>
#include <iostream>

// Simple GLFW window demo
// This creates a basic window without any graphics API

static void errorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

int main() {
    // Initialize GLFW
    glfwSetErrorCallback(errorCallback);
    
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Create window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);  // We're not using OpenGL
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    
    GLFWwindow* window = glfwCreateWindow(800, 600, "Minimal GLFW Window", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwSetKeyCallback(window, keyCallback);

    std::cout << "GLFW window created successfully!" << std::endl;
    std::cout << "Press ESC to close the window." << std::endl;
    std::cout << "Note: This is a basic window. WebGPU triangle rendering would require more complex setup." << std::endl;

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        
        // Here we would normally do WebGPU rendering
        // For now, this just keeps the window open
        
        glfwWaitEvents(); // Wait for events to save CPU
    }

    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
    
    std::cout << "Application terminated successfully." << std::endl;
    return 0;
}
