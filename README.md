# 🎮 RS Engine WebGPU

**Physics Simulation Engine** - 웹과 네이티브에서 동일한 코드로 실행되는 고성능 유체/천 시뮬레이션 엔진

## ✨ 특징

- 🌐 **크로스플랫폼**: 하나의 코드베이스로 웹과 네이티브 앱 개발
- ⚡ **WebGPU**: 최신 그래픽 API로 GPU 가속 물리 시뮬레이션
- 🌊 **유체 시뮬레이션**: SPH(Smoothed Particle Hydrodynamics) 알고리즘
- 🧵 **천 시뮬레이션**: PBD(Position Based Dynamics) 알고리즘
- 🔥 **Hot Reload**: 개발 중 실시간 코드 변경 감지
- 🎯 **단일 소스**: 95% 이상의 코드 공유
- 📊 **적응형 품질**: 플랫폼 성능에 따른 자동 최적화

## 🚀 빠른 시작

### 유체 시뮬레이션 데모

```bash
# 웹에서 유체 시뮬레이션 실행 (자동 리로드)
npm run dev:fluid

# 네이티브에서 유체 시뮬레이션 실행
npm run native:fluid
```

### 기본 개발 (삼각형 렌더링)

```bash
# 개발 모드 (자동 리로드)
npm run dev

# 빌드만
npm run build

# 서버만 실행
npm run preview
```

### 네이티브 개발

#### Unix/macOS
```bash
# 네이티브 빌드 & 실행
npm run native

# 또는 직접
./run_native.sh
```

#### Windows
```powershell
# Dawn 라이브러리 빌드 (최초 1회)
npm run build:dawn:windows

# 애플리케이션 빌드 & 실행
npm run native:windows

# 또는 Command Prompt에서
npm run build:dawn:win
npm run native:win
```

> 📋 **Windows 사용자**: 자세한 설정 가이드는 [WINDOWS_SETUP.md](./WINDOWS_SETUP.md)를 참조하세요.

## 📦 사용 가능한 명령어

### 물리 시뮬레이션
| 명령어 | 설명 |
|--------|------|
| `npm run dev:fluid` | 🌊 유체 시뮬레이션 개발 모드 |
| `npm run native:fluid` | 🌊 유체 시뮬레이션 네이티브 실행 |
| `npm run build:web:fluid` | 🌊 유체 시뮬레이션 웹 빌드 |
| `npm run build:native:fluid` | 🌊 유체 시뮬레이션 네이티브 빌드 |
| `npm run preview:fluid` | 🌊 유체 시뮬레이션 서버 실행 |

### 기본 개발
| 명령어 | 설명 |
|--------|------|
| `npm run dev` | 🔥 기본 개발 모드 (삼각형 렌더링) |
| `npm run build` | 🔨 웹 버전 빌드 |
| `npm run preview` | 🌍 빌드된 파일 서버 실행 |
| `npm run native` | 🖥️ 네이티브 빌드 & 실행 |
| `npm run clean` | 🧹 모든 빌드 파일 정리 |

### Windows 전용
| 명령어 | 설명 |
|--------|------|
| `npm run build:dawn:windows` | 🌅 Dawn 라이브러리 빌드 (PowerShell) |
| `npm run build:dawn:win` | 🌅 Dawn 라이브러리 빌드 (Batch) |
| `npm run native:windows` | 🖥️ 네이티브 빌드 & 실행 (PowerShell) |
| `npm run native:win` | 🖥️ 네이티브 빌드 & 실행 (Batch) |

## 🏗️ 프로젝트 구조 (95% 코드 공유 아키텍처)

```
rs_engine_webgpu/
├── engine/                          # 95% 공통 엔진 코드
│   ├── core/                       # 100% 공통 핵심 시스템
│   │   ├── Config.h               # 플랫폼별 설정 통합 관리
│   │   ├── Application.h          # 공통 애플리케이션 기반
│   │   ├── ecs/                   # Entity Component System
│   │   ├── math/                  # 수학 라이브러리
│   │   └── memory/                # 메모리 관리
│   ├── rendering/                  # 95% 공통 렌더링
│   │   ├── WebGPURenderer.h       # 통합 WebGPU 렌더러
│   │   ├── shaders/               # 셰이더 관리
│   │   ├── scene/                 # 씬 그래프
│   │   └── materials/             # 머티리얼 시스템
│   ├── physics/                    # 100% 공통 물리 엔진
│   │   ├── PhysicsWorld.h         # 물리 월드 관리
│   │   ├── fluid/SPHSimulation.h  # SPH 유체 시뮬레이션
│   │   ├── cloth/PBDCloth.h       # PBD 천 시뮬레이션
│   │   └── collision/             # 충돌 감지
│   ├── platform/                   # 5% 플랫폼별 코드
│   │   ├── WebApplication.h       # 웹 플랫폼 구현
│   │   └── NativeApplication.h    # 네이티브 플랫폼 구현
│   └── utils/                      # 100% 공통 유틸리티
├── shaders/                         # 100% 공통 WGSL 셰이더
│   ├── common/types.wgsl           # 공통 데이터 구조
│   ├── compute/fluid/              # 유체 시뮬레이션 셰이더
│   └── compute/cloth/              # 천 시뮬레이션 셰이더
├── apps/                            # 애플리케이션 구현
│   ├── viewer/                     # 기본 삼각형 렌더링 데모
│   ├── fluid_demo/                 # 유체 시뮬레이션 데모
│   └── cloth_demo/                 # 천 시뮬레이션 데모 (예정)
├── config/                          # 런타임 설정 파일
└── extern/dawn/                     # Dawn WebGPU 구현
```

## 🛠️ 개발 가이드

### 아키텍처 원칙

1. **95% 코드 공유**: `rs_engine::EngineConfig`를 통한 플랫폼 설정 관리
2. **GPU 우선**: 모든 물리 시뮬레이션은 WebGPU 컴퓨트 셰이더에서 실행
3. **통합 API**: 플랫폼에 관계없이 동일한 인터페이스 사용
4. **적응형 품질**: 런타임에 플랫폼 성능에 따른 자동 조절

### 플랫폼별 제한사항

| 플랫폼 | 최대 파티클 수 | 최대 버퍼 크기 | 워크그룹 크기 | 고급 기능 |
|--------|---------------|---------------|---------------|----------|
| **웹** | 32,768 | 64MB | 64 | 비활성화 |
| **네이티브** | 262,144 | 512MB | 128 | 활성화 |

### 새로운 물리 시뮬레이션 추가

```cpp
// engine/physics/my_simulation/MySimulation.h
class MySimulation {
    WebGPURenderer* renderer;
    uint32_t particleCount;

public:
    MySimulation(WebGPURenderer* r) : renderer(r) {
        // 플랫폼별 설정 자동 적용
        auto limits = EngineConfig::getLimits();
        particleCount = limits.maxParticles / 4;
    }

    void setQuality(float quality) {
        // 품질에 따른 파티클 수 조절
        auto limits = EngineConfig::getLimits();
        particleCount = static_cast<uint32_t>(limits.maxParticles * quality);
    }

    void update(float deltaTime) {
        // 100% 공통 알고리즘
        // 플랫폼별 차이는 설정값으로만 구분
    }
};
```

### WGSL 셰이더 개발

```wgsl
// shaders/compute/my_simulation/kernel.wgsl
// 플랫폼별 정의는 C++에서 자동 주입:
// #define MAX_PARTICLES 32768
// #define WORKGROUP_SIZE 64

@compute @workgroup_size(WORKGROUP_SIZE, 1, 1)
fn main(@builtin(global_invocation_id) id: vec3u) {
    let particle_id = id.x;
    if (particle_id >= MAX_PARTICLES) { return; }

    // 100% 동일한 알고리즘
    // 플랫폼 차이는 설정값으로만 처리
}
```

## ✅ 현재 상태

### 완료된 기능
- [x] 🌐 웹/네이티브 통합 빌드 시스템
- [x] ⚙️ 95% 코드 공유 아키텍처 (EngineConfig)
- [x] 🎨 WebGPU 삼각형 렌더링 (웹/네이티브)
- [x] 🖼️ 통합 WebGPU 렌더러 (WebGPURenderer)
- [x] 🌊 SPH 유체 시뮬레이션 기반 구조
- [x] 🧵 PBD 천 시뮬레이션 기반 구조
- [x] 🔧 npm 기반 개발 워크플로우
- [x] 🚀 자동 리로드 개발 서버
- [x] 📊 적응형 품질 관리 시스템
- [x] 💻 WGSL 컴퓨트 셰이더 인프라

### 개발 중인 기능
- [ ] 🌊 SPH 유체 시뮬레이션 구현 완성
- [ ] 🧵 PBD 천 시뮬레이션 구현 완성
- [ ] 🎯 컴퓨트 셰이더 파이프라인 연결
- [ ] 🖼️ 파티클 렌더링 시스템
- [ ] ⌨️ 입력 시스템 통합
- [ ] 🔍 충돌 감지 시스템

## 🌐 웹 지원

- ✅ Chrome/Edge (WebGPU 지원)
- ✅ Firefox (flag 활성화 필요)
- ❌ Safari (개발 중)

## 📋 요구사항

### 공통
- **웹**: Emscripten, 현대적인 브라우저
- **Node.js**: npm 스크립트 실행용
- **Python 3**: 개발 서버용

### Unix/macOS 네이티브
- **Dawn WebGPU**: Dawn 라이브러리
- **GLFW**: 창 관리
- **CMake**: 빌드 시스템

### Windows 네이티브
- **Visual Studio 2019+**: C++ 빌드 도구
- **CMake 3.24+**: 빌드 시스템
- **Git**: 서브모듈 관리
- **Python 3.7+**: Dawn 빌드용

> 📖 **Windows 사용자**: 자세한 설정 방법은 [WINDOWS_SETUP.md](./WINDOWS_SETUP.md)를 참조하세요.

---

**🎮 즐거운 개발 되세요!** WebGPU로 최고의 성능을 경험해보세요.
