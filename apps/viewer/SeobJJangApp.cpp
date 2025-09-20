#include "SeobJJangApp.h"

SeobJJangApp::SeobJJangApp() {}

SeobJJangApp::~SeobJJangApp() {}

bool SeobJJangApp::onInit()
{
    BaseApp::onInit();
    std::cout << "🎯 SeobJJang App initialized successfully!" << std::endl;
    std::cout << "🎮 Controls:" << std::endl;
    std::cout << "   - Press ESC to close" << std::endl;
    return true;
}

void SeobJJangApp::update(const float deltaTime)
{
    BaseApp::update(deltaTime);
}

void SeobJJangApp::draw()
{
    BaseApp::draw();
}
