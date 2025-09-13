#!/usr/bin/env python3
"""
WebGPU Development Server
Enhanced HTTP server with better MIME types and CORS support for WebGPU development
"""

import http.server
import socketserver
import os
import sys
from urllib.parse import urlparse

class WebGPUHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    def end_headers(self):
        # Add CORS headers for WebGPU
        self.send_header('Cross-Origin-Embedder-Policy', 'require-corp')
        self.send_header('Cross-Origin-Opener-Policy', 'same-origin')
        super().end_headers()
    
    def guess_type(self, path):
        mimetype, encoding = super().guess_type(path)
        
        # Enhanced MIME types for WebAssembly and WebGPU
        if path.endswith('.wasm'):
            return 'application/wasm', encoding
        elif path.endswith('.js'):
            return 'application/javascript', encoding
        elif path.endswith('.html'):
            return 'text/html', encoding
        
        return mimetype, encoding
    
    def log_message(self, format, *args):
        # Cleaner log format
        message = format % args
        if '200' in message:
            print(f"✅ {message}")
        elif '404' in message:
            print(f"❌ {message}")
        else:
            print(f"ℹ️  {message}")

def run_server(port=3377, directory=None):
    if directory:
        os.chdir(directory)
    
    handler = WebGPUHTTPRequestHandler
    
    with socketserver.TCPServer(("", port), handler) as httpd:
        print(f"🌍 WebGPU Development Server")
        print(f"   📂 Serving: {os.getcwd()}")
        print(f"   🌐 URL: http://localhost:{port}")
        print(f"   🎮 WebGPU ready with proper headers")
        print(f"   ⏹️  Press Ctrl+C to stop")
        print()
        
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\n👋 Server stopped")

if __name__ == "__main__":
    port = 3377
    directory = None
    
    if len(sys.argv) > 1:
        port = int(sys.argv[1])
    if len(sys.argv) > 2:
        directory = sys.argv[2]
    
    run_server(port, directory)
