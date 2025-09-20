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

# Function to quickly copy HTML file
copy_html() {
    echo "🔄 HTML file changed, copying..."
    cp apps/viewer/index.html build_web/apps/viewer/index.html
    echo "✅ HTML updated at $(date)"
}

# Function to handle file changes
handle_change() {
    local changed_file="$1"
    if [[ "$changed_file" == *"index.html"* ]]; then
        copy_html
    else
        build_web
    fi
}

# Initial build
build_web

# Kill any existing server on port 3377
./scripts/kill-port.sh 3377

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
    fswatch -r apps/ engine/ | while read changed_file; do
        echo "🔄 File change detected: $changed_file"
        handle_change "$changed_file"
    done
else
    echo "💡 Install fswatch for auto-reload: brew install fswatch"
    echo "   For now, manually run 'npm run build' when you change files"
    echo "   Or use 'npm run copy-html' to update HTML quickly"
    wait $SERVER_PID
fi

# Cleanup
kill $SERVER_PID 2>/dev/null
