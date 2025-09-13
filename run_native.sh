#!/bin/bash

# Build script for WebGPU Triangle Demo - Native Version

echo "🖥️  Building WebGPU Triangle Demo (Native Version)..."

# Configure and build for native
cmake -S . -B build -DBUILD_WEB=OFF
cmake --build build --target viewer

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo "🚀 Running unified viewer (native)..."
    echo "📝 This uses the new unified codebase with rs_engine"
    echo "🔧 Press ESC to close the window"
    ./build/apps/viewer/viewer
else
    echo "❌ Build failed!"
    echo "💡 Trying fallback to viewer_simple..."
    cmake --build build --target viewer_simple
    if [ $? -eq 0 ]; then
        echo "✅ Fallback build successful!"
        ./build/apps/viewer/viewer_simple
    else
        echo "❌ All builds failed!"
        exit 1
    fi
fi
