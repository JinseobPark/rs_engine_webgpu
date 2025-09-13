#!/bin/bash

# Build scrip        echo "🌍 Open your browser and go to: http://localhost:3377"
        python3 -m http.server 3377for WebGPU Triangle Demo - Web Version

echo "🌐 Building WebGPU Triangle Demo (Web Version)..."

# Configure and build for web
emcmake cmake -S . -B build_web -DBUILD_WEB=ON
cmake --build build_web --target viewer

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo "🚀 Starting web server..."
    cd build_web/apps/viewer
    echo "🌍 Open your browser and go to: http://localhost:3377"
    echo "📝 Make sure your browser supports WebGPU"
    echo "🔧 Press Ctrl+C to stop the server"
    echo "💡 Using new unified codebase with rs_engine"
    python3 -m http.server 3377
else
    echo "❌ Build failed!"
    echo "💡 Trying fallback to viewer_web..."
    cmake --build build_web --target viewer_web
    if [ $? -eq 0 ]; then
        echo "✅ Fallback build successful!"
        cd build_web/apps/viewer
        echo "🌍 Open your browser and go to: http://localhost:3377"
        python3 -m http.server 3377
    else
        echo "❌ All builds failed!"
        exit 1
    fi
fi
