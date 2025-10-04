# Application System

## 책임
- 플랫폼 초기화 (Window/Canvas)
- WebGPU 디바이스 및 서피스 관리
- 저수준 이벤트 처리 (GLFW poll)
- 플랫폼별 리소스 관리

## 우선순위
**Priority: -100** (가장 먼저 초기화)

## 플랫폼 지원
- **Native**: GLFW + Dawn WebGPU ✅
- **Web**: HTML5 Canvas + Emscripten WebGPU ✅

## 주요 API

```cpp
class ApplicationSystem : public IEngineSystem {
    // WebGPU 리소스
    wgpu::Device& getDevice();
    wgpu::Surface& getSurface();
    wgpu::Instance& getInstance();
    
    // 윈도우 정보
    uint32_t getWindowWidth() const;
    uint32_t getWindowHeight() const;
    
    // 애플리케이션 상태
    bool shouldClose() const;
    
    // Native 전용
    GLFWwindow* getWindow();
};
```

## 이벤트 전달

ApplicationSystem은 저수준 이벤트를 받아 InputSystem으로 전달합니다:

```cpp
// GLFW 콜백 → InputSystem
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto* inputSystem = engine->getSystem<InputSystem>();
    if (inputSystem) {
        inputSystem->updateKeyState(key, pressed);
    }
}
```

## 의존성
- **없음** (최하위 시스템)

## 사용 예시

```cpp
// Engine이 자동으로 추가
Engine engine;
engine.initialize();  // ApplicationSystem 자동 추가됨

// 직접 접근 (필요 시)
auto* appSystem = engine.getSystem<ApplicationSystem>();
uint32_t width = appSystem->getWindowWidth();
```

## 노트
- ApplicationSystem은 **Engine이 자동으로 추가**합니다
- 앱 개발자는 직접 사용할 필요가 거의 없음
- Engine 인터페이스를 통해 간접적으로 사용
