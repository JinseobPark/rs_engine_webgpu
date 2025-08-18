# engine/CMakeLists.txt
add_library(rs_engine_webgpu STATIC
  core/input.cpp core/input.hpp
  gfx/rhi.cpp gfx/rhi.hpp
  platform/native/native_platform.cpp
  platform/web/web_platform.cpp
)

if(BUILD_WEB)
  # Emscripten 빌드 플래그
  target_compile_options(rs_engine_webgpu PRIVATE
    "-sUSE_WEBGPU=1" "-sALLOW_MEMORY_GROWTH=1")
else()
  # Dawn 헤더/라이브러리 연결 (GN 빌드물 경로를 캐시 변수로)
  set(DAWN_ROOT "${CMAKE_SOURCE_DIR}/extern/dawn" CACHE PATH "")
  set(DAWN_BUILD "${DAWN_ROOT}/out/Release" CACHE PATH "")
  target_include_directories(rs_engine_webgpu PUBLIC
    ${DAWN_ROOT}/include
    ${DAWN_BUILD}/gen/include)
  target_link_directories(rs_engine_webgpu PUBLIC ${DAWN_BUILD})
  # GLFW는 앱에서 링크
endif()
