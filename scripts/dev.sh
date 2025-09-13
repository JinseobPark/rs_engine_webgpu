#!/bin/bash

# Development script with auto-reload
echo "🚀 Starting development mode..."

# Function to build web
build_web() {
    echo "🔨 Building web version..."
    emcmake cmake -S . -B build_web -DBUILD_WEB=ON > /dev/null 2>&1
    cmake --build build_web --target viewer > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo "✅ Build successful at $(date)"
    else
        echo "❌ Build failed at $(date)"
    fi
}

# Initial build
build_web

# Start server in background
echo "🌍 Starting web server at http://localhost:3377"
cd build_web/apps/viewer
python3 -m http.server 3377 > /dev/null 2>&1 &
SERVER_PID=$!

# Go back to root
cd ../../..

echo "👀 Watching for file changes..."
echo "   - Press Ctrl+C to stop"
echo "   - Files being watched: apps/, engine/"

# Watch for changes (macOS)
if command -v fswatch >/dev/null 2>&1; then
    fswatch -o apps/ engine/ | while read f; do
        echo "🔄 File change detected, rebuilding..."
        build_web
    done
else
    echo "💡 Install fswatch for auto-reload: brew install fswatch"
    echo "   For now, manually run 'npm run build' when you change files"
    wait $SERVER_PID
fi

# Cleanup
kill $SERVER_PID 2>/dev/null
