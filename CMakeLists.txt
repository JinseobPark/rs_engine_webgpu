# CMakeLists.txt (루트)
cmake_minimum_required(VERSION 3.24)
project(rs_engine_webgpu CXX)

option(BUILD_WEB "Build Web (Emscripten + WebGPU)" OFF)

add_subdirectory(engine)
add_subdirectory(apps/viewer)
