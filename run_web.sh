#!/bin/bash

# Build script for WebGPU Triangle Demo - Web Version

echo "🌐 Building WebGPU Triangle Demo (Web Version)..."

# Configure and build for web
emcmake cmake -S . -B build_web -DBUILD_WEB=ON
cmake --build build_web

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo "🚀 Starting web server..."
    cd build_web/apps/viewer
    echo "🌍 Open your browser and go to: http://localhost:8080"
    echo "📝 Make sure your browser supports WebGPU"
    echo "🔧 Press Ctrl+C to stop the server"
    python3 -m http.server 8080
else
    echo "❌ Build failed!"
    exit 1
fi
