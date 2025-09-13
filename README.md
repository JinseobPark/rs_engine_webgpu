# 🎮 RS Engine WebGPU

크로스플랫폼 WebGPU 엔진 - 웹과 네이티브에서 동일한 코드로 실행되는 고성능 그래픽 애플리케이션

## ✨ 특징

- 🌐 **크로스플랫폼**: 하나의 코드베이스로 웹과 네이티브 앱 개발
- ⚡ **WebGPU**: 최신 그래픽 API로 고성능 렌더링
- 🔥 **Hot Reload**: 개발 중 실시간 코드 변경 감지
- 🎯 **단일 소스**: 95% 이상의 코드 공유

## 🚀 빠른 시작

### 웹 개발 (권장)

```bash
# 개발 모드 (자동 리로드)
npm run dev

# 빌드만
npm run build

# 서버만 실행
npm run preview
```

### 네이티브 개발

```bash
# 네이티브 빌드 & 실행
npm run native

# 또는 직접
./run_native.sh
```

## 📦 사용 가능한 명령어

| 명령어 | 설명 |
|--------|------|
| `npm run dev` | 🔥 개발 모드 (자동 리로드) |
| `npm run build` | 🔨 웹 버전 빌드 |
| `npm run preview` | 🌍 빌드된 파일 서버 실행 |
| `npm run native` | 🖥️ 네이티브 개발 |
| `npm run clean` | 🧹 모든 빌드 파일 정리 |
| `npm run help` | ❓ 도움말 |

## 🏗️ 프로젝트 구조

```
rs_engine_webgpu/
├── engine/                 # 공통 엔진 코드
│   ├── core/              # 핵심 기능
│   └── platform/          # 플랫폼별 구현
├── apps/viewer/           # 샘플 애플리케이션
│   ├── main.cpp          # 통합 메인 파일
│   ├── TriangleApp.h     # 앱 로직
│   └── index.html        # 웹용 HTML
├── scripts/              # 개발 도구
│   ├── dev.sh           # 개발 모드 스크립트
│   └── dev-server.py    # WebGPU 최적화 서버
└── extern/dawn/         # Dawn WebGPU 구현
```

## 🛠️ 개발 가이드

### 새로운 기능 추가

1. **공통 로직**: `engine/core/` 또는 `apps/viewer/TriangleApp.h`에 구현
2. **플랫폼별 코드**: `#ifdef __EMSCRIPTEN__`로 분기 처리
3. **테스트**: `npm run dev`로 웹에서, `npm run native`로 네이티브에서 확인

### 코드 구조

```cpp
// apps/viewer/TriangleApp.h
class TriangleApp : public BaseApp {
public:
    bool onInit() override {
        // 공통 초기화 로직
        return true;
    }
    
    void update(float deltaTime) override {
        // 공통 업데이트 로직
    }
    
    void draw() override {
        // 공통 렌더링 로직
    }
};
```

## ✅ 현재 상태

### 작동하는 기능
- [x] 🌐 웹/네이티브 통합 빌드 시스템
- [x] 🎨 WebGPU 삼각형 렌더링 (웹)
- [x] 🖼️ GLFW 창 관리 (네이티브)
- [x] 🔧 npm 기반 개발 워크플로우
- [x] 🚀 자동 리로드 개발 서버
- [x] 📱 크로스플랫폼 코드 공유

### 개발 중인 기능
- [ ] 🎯 네이티브 WebGPU 렌더링 완성
- [ ] 💾 컴퓨트 셰이더 지원
- [ ] 🌊 물리 시뮬레이션 엔진
- [ ] 🖼️ 텍스처 시스템
- [ ] ⌨️ 입력 시스템

## 🌐 웹 지원

- ✅ Chrome/Edge (WebGPU 지원)
- ✅ Firefox (flag 활성화 필요)
- ❌ Safari (개발 중)

## 📋 요구사항

- **웹**: Emscripten, 현대적인 브라우저
- **네이티브**: Dawn WebGPU, GLFW, CMake
- **공통**: Node.js, Python 3

---

**🎮 즐거운 개발 되세요!** WebGPU로 최고의 성능을 경험해보세요.
