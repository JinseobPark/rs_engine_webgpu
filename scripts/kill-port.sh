#!/bin/bash

# Kill process running on specified port
PORT=${1:-3377}

echo "🔌 Killing processes on port $PORT..."

# Method 1: Using lsof (most reliable)
if command -v lsof >/dev/null 2>&1; then
    PIDS=$(lsof -ti:$PORT 2>/dev/null)
    if [ ! -z "$PIDS" ]; then
        echo "Found processes: $PIDS"
        echo "$PIDS" | xargs kill -9 2>/dev/null || true
        echo "✅ Killed processes on port $PORT"
    else
        echo "ℹ️  No processes found on port $PORT"
    fi
else
    # Method 2: Fallback using pkill
    pkill -f "python3 -m http.server $PORT" 2>/dev/null || true
    echo "✅ Attempted to kill HTTP server on port $PORT"
fi

# Wait a moment for cleanup
sleep 1
echo "🎯 Port $PORT is now available"