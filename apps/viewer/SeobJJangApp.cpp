#include "SeobJJangApp.h"

SeobJJangApp::SeobJJangApp() {}

SeobJJangApp::~SeobJJangApp() {}

bool SeobJJangApp::onInit()
{
    std::cout << "🎯 SeobJJang App initialized successfully!" << std::endl;
    std::cout << "🎮 Controls:" << std::endl;
    std::cout << "   - Press ESC to close" << std::endl;
    return true;
}

void SeobJJangApp::update(const float deltaTime)
{
    // 여기에 공통 업데이트 로직 추가
    BaseApp::update(deltaTime);
}

void SeobJJangApp::draw()
{
    // 여기에 공통 렌더링 로직 추가
    BaseApp::draw();
}
