#!/usr/bin/env python3
"""
Simple HTTPS server for WebGPU testing
Creates a self-signed certificate and serves files over HTTPS
"""

import http.server
import ssl
import os
import subprocess
import socketserver
from pathlib import Path

def create_self_signed_cert():
    """Create a self-signed certificate for HTTPS"""
    cert_file = "server.crt"
    key_file = "server.key"

    if os.path.exists(cert_file) and os.path.exists(key_file):
        print("📜 Using existing certificate")
        return cert_file, key_file

    print("🔐 Creating self-signed certificate...")

    # Create self-signed certificate
    subprocess.run([
        "openssl", "req", "-new", "-x509", "-keyout", key_file,
        "-out", cert_file, "-days", "365", "-nodes",
        "-subj", "/C=US/ST=CA/L=localhost/O=WebGPU-Test/CN=localhost"
    ], check=True)

    print("✅ Certificate created successfully")
    return cert_file, key_file

def start_https_server(port=3443, directory="."):
    """Start HTTPS server"""

    # Change to the specified directory
    os.chdir(directory)

    try:
        cert_file, key_file = create_self_signed_cert()
    except subprocess.CalledProcessError:
        print("❌ Failed to create certificate. Make sure OpenSSL is installed.")
        print("💡 On macOS: brew install openssl")
        print("💡 On Ubuntu: sudo apt-get install openssl")
        return
    except FileNotFoundError:
        print("❌ OpenSSL not found. Please install OpenSSL first.")
        print("💡 On macOS: brew install openssl")
        print("💡 On Ubuntu: sudo apt-get install openssl")
        return

    # Create HTTPS server
    Handler = http.server.SimpleHTTPRequestHandler

    with socketserver.TCPServer(("", port), Handler) as httpd:
        # Wrap socket with SSL
        context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        context.load_cert_chain(cert_file, key_file)
        httpd.socket = context.wrap_socket(httpd.socket, server_side=True)

        print(f"🚀 HTTPS Server running at: https://localhost:{port}")
        print(f"📁 Serving directory: {Path(directory).absolute()}")
        print("🔒 Using self-signed certificate (browser will warn - click 'Advanced' and 'Proceed')")
        print("⚠️  For WebGPU testing, ignore certificate warnings in browser")
        print("🛑 Press Ctrl+C to stop")

        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\n🛑 Server stopped")

if __name__ == "__main__":
    import sys

    port = 3443
    directory = "build_web/apps/viewer"

    if len(sys.argv) > 1:
        directory = sys.argv[1]
    if len(sys.argv) > 2:
        port = int(sys.argv[2])

    print("🌐 WebGPU HTTPS Test Server")
    print("=" * 40)

    start_https_server(port, directory)