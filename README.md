# WebGPU Triangle Demo

A cross-platform WebGPU triangle rendering application with both native (Dawn) and web (Emscripten) versions.

## 🏗️ Project Structure

```
rs_engine_webgpu/
├── extern/dawn/              # Dawn WebGPU library (built)
├── apps/viewer/              # Main application
│   ├── main_native.cpp       # Native version (Dawn WebGPU)
│   ├── main_web.cpp          # Web version (Emscripten)
│   ├── main.cpp              # Simple GLFW baseline
│   ├── index.html            # Web template
│   └── CMakeLists.txt        # Build configuration
├── build/                    # Native build directory
├── build_web/                # Web build directory
├── run_native.sh            # Quick native build & run
└── run_web.sh               # Quick web build & serve
```

## ✅ What's Working

### Native Version (Simple GLFW)
- ✅ **GLFW Window Management**: Creates and manages window successfully
- ✅ **Cross-platform Build**: Builds on macOS with proper framework linking
- ✅ **Event Handling**: ESC key to close window

### Web Version (WebGPU)
- ✅ **Emscripten Build**: Successfully compiles to WebAssembly
- ✅ **WebGPU Integration**: Uses modern WebGPU APIs
- ✅ **Canvas Rendering**: Renders to HTML5 canvas element
- ✅ **Triangle Rendering**: Displays green triangle using WGSL shaders
- ✅ **Local Server**: Includes HTTP server for testing

### Dawn WebGPU Library
- ✅ **Successfully Built**: All 185+ libraries compiled
- ✅ **Modern APIs**: Updated to latest Dawn structure
- ✅ **macOS Support**: Metal backend integration

## 🚀 Quick Start

### Native Version (Simple)
```bash
./run_native.sh
```

### Web Version (WebGPU Triangle)
```bash
./run_web.sh
# Then open http://localhost:8080 in a WebGPU-compatible browser
```

## 🔧 Manual Build Instructions

### Prerequisites
- **macOS**: Xcode Command Line Tools
- **CMake**: 3.16+
- **Emscripten**: For web builds
- **WebGPU Browser**: Chrome/Edge with WebGPU enabled

### Native Build
```bash
cmake -S . -B build -DBUILD_WEB=OFF
cmake --build build --target viewer_simple
./build/apps/viewer/viewer_simple
```

### Web Build
```bash
emcmake cmake -S . -B build_web -DBUILD_WEB=ON
cmake --build build_web
cd build_web/apps/viewer
python3 -m http.server 8080
# Open http://localhost:8080
```

## 🌐 Browser Requirements

For the web version, you need a browser with WebGPU support:
- **Chrome/Chromium**: Enable `chrome://flags/#enable-unsafe-webgpu`
- **Edge**: Enable WebGPU flags
- **Firefox**: Experimental support

## 📋 Current Status

### Working Features ✅
- [x] Dawn WebGPU library built successfully
- [x] Native GLFW window creation and management
- [x] Web WebGPU triangle rendering with shaders
- [x] Cross-platform build system (Native/Web)
- [x] Modern WebGPU C++ API usage
- [x] WGSL shader compilation
- [x] Canvas surface creation for web
- [x] Build automation scripts

### Potential Improvements 🔄
- [ ] Full Dawn WebGPU native triangle rendering
- [ ] Texture and advanced rendering features
- [ ] Input handling for interactive demos
- [ ] More complex geometry and shaders
- [ ] Resource management optimization

## 🧩 Architecture

### Native Version
- **GLFW**: Window management and input
- **Dawn**: Google's WebGPU implementation
- **Metal**: macOS graphics backend
- **C++17**: Modern C++ features

### Web Version
- **Emscripten**: C++ to WebAssembly compilation
- **WebGPU**: Browser's native WebGPU implementation
- **WGSL**: WebGPU Shading Language
- **HTML5 Canvas**: Rendering target

## 🔍 Technical Details

### Shader Code (WGSL)
```wgsl
// Vertex Shader
@vertex
fn vs_main(@builtin(vertex_index) vertexIndex: u32) -> @builtin(position) vec4f {
    var pos = array<vec2f, 3>(
        vec2f( 0.0,  0.5),
        vec2f(-0.5, -0.5),
        vec2f( 0.5, -0.5)
    );
    return vec4f(pos[vertexIndex], 0.0, 1.0);
}

// Fragment Shader
@fragment
fn fs_main() -> @location(0) vec4f {
    return vec4f(0.2, 0.8, 0.2, 1.0);  // Green color
}
```

### Build Flags
- **Native**: Links to Dawn, GLFW, Metal frameworks
- **Web**: `-sUSE_WEBGPU=1`, `-sASYNCIFY=1` for async operations

## 📚 Development Notes

The project demonstrates both approaches to WebGPU development:

1. **Native Development**: Uses Dawn (Google's WebGPU implementation) for desktop applications
2. **Web Development**: Uses browser's native WebGPU for web applications

Both versions share similar WebGPU concepts but have different surface creation and build requirements.

## 🐛 Known Issues

- Dawn native version requires extensive library linking (185+ libraries)
- Web version needs WebGPU-enabled browser
- Some WebGPU APIs differ between Dawn and browser implementations

## 🎯 Next Steps

1. **Complete Dawn Integration**: Resolve all library dependencies for native WebGPU triangle
2. **Advanced Features**: Add textures, uniforms, and more complex rendering
3. **Input System**: Add mouse and keyboard interaction
4. **Performance**: Optimize rendering loop and resource management
