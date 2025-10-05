#!/bin/bash

# Build script for WebGPU Triangle Demo - Web Version

echo "[INFO] Building WebGPU Triangle Demo (Web Version)..."

# Configure and build for web
emcmake cmake -S . -B build_web -DBUILD_WEB=ON
cmake --build build_web --target viewer

if [ $? -eq 0 ]; then
    echo "[SUCCESS] Build successful!"
    echo "[INFO] Starting web server..."
    cd build_web/apps/viewer
    echo "[INFO] Open your browser and go to: http://localhost:3377"
    echo "[INFO] Make sure your browser supports WebGPU"
    echo "[INFO] Press Ctrl+C to stop the server"
    echo "[INFO] Using new unified codebase with rs_engine"
    python3 -m http.server 3377
else
    echo "[ERROR] Build failed!"
    echo "[INFO] Trying fallback to viewer_web..."
    cmake --build build_web --target viewer_web
    if [ $? -eq 0 ]; then
        echo "[SUCCESS] Fallback build successful!"
        cd build_web/apps/viewer
        echo "[INFO] Open your browser and go to: http://localhost:3377"
        python3 -m http.server 3377
    else
        echo "[ERROR] All builds failed!"
        exit 1
    fi
fi
