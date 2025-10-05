#!/bin/bash
      echo "[INFO] This uses the new unified codebase with rs_engine"
    echo "[INFO] Press ESC to close the window"echo "[INFO] Running unified viewer (native)..."
    echo "[INFO] This uses the new unified codebase with rs_engine"
    echo "[INFO] Press ESC to close the window"Build script for WebGPU Triangle Demo - Native Version

echo "[INFO] Building WebGPU Triangle Demo (Native Version)..."

# Configure and build for native
cmake -S . -B build -DBUILD_WEB=OFF
cmake --build build --target viewer

if [ $? -eq 0 ]; then
    echo "[SUCCESS] Build successful!"
    echo "[INFO] Running unified viewer (native)..."
    echo "📝 This uses the new unified codebase with rs_engine"
    echo "🔧 Press ESC to close the window"
    ./build/apps/viewer/viewer
else
    echo "[ERROR] Build failed!"
    echo "[INFO] Trying fallback to viewer_simple..."
    cmake --build build --target viewer_simple
    if [ $? -eq 0 ]; then
        echo "[SUCCESS] Fallback build successful!"
        ./build/apps/viewer/viewer_simple
    else
        echo "[ERROR] All builds failed!"
        exit 1
    fi
fi
