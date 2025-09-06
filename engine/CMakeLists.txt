# engine/CMakeLists.txt
add_library(rs_engine_webgpu INTERFACE)

if(BUILD_WEB)
  # Emscripten 빌드 플래그
  target_compile_options(rs_engine_webgpu INTERFACE
    "-sUSE_WEBGPU=1" "-sALLOW_MEMORY_GROWTH=1")
else()
  # Dawn 헤더/라이브러리 연결 (CMake 빌드물 경로)
  set(DAWN_ROOT "${CMAKE_SOURCE_DIR}/extern/dawn" CACHE PATH "")
  set(DAWN_BUILD "${DAWN_ROOT}/build" CACHE PATH "")
  target_include_directories(rs_engine_webgpu INTERFACE
    ${DAWN_ROOT}/include
    ${DAWN_BUILD}/gen/include)
  target_link_directories(rs_engine_webgpu INTERFACE ${DAWN_BUILD}/src/dawn)
endif()
