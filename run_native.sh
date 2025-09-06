#!/bin/bash

# Build script for WebGPU Triangle Demo - Native Version

echo "🖥️  Building WebGPU Triangle Demo (Native Version)..."

# Configure and build for native
cmake -S . -B build -DBUILD_WEB=OFF
cmake --build build --target viewer_simple

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo "🚀 Running native viewer..."
    echo "📝 This is a simple GLFW window demo"
    echo "🔧 Press ESC to close the window"
    ./build/apps/viewer/viewer_simple
else
    echo "❌ Build failed!"
    exit 1
fi
