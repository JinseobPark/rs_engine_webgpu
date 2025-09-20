# engine/CMakeLists.txt

# Create the engine library
if(EMSCRIPTEN)
    add_library(rs_engine_webgpu STATIC
        core/Application.cpp
        platform/WebApplication.cpp
    )
else()
    add_library(rs_engine_webgpu STATIC
        core/Application.cpp
        platform/NativeApplication.cpp
    )
endif()

# Set C++17 for compatibility
set_target_properties(rs_engine_webgpu PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Platform-specific setup
if(EMSCRIPTEN)
    # Web version setup
    target_compile_options(rs_engine_webgpu PRIVATE 
        -sUSE_WEBGPU=1
    )
    
    target_link_options(rs_engine_webgpu INTERFACE
        -sUSE_WEBGPU=1
        -sASYNCIFY=1
        -sALLOW_MEMORY_GROWTH=1
        --bind
        -sTOTAL_MEMORY=256MB
        -sEXPORTED_RUNTIME_METHODS=["ccall","cwrap"]
        -lhtml5
    )
else()
    # Native version setup using Dawn WebGPU
    set(DAWN_ROOT "${CMAKE_SOURCE_DIR}/extern/dawn")
    set(DAWN_BUILD "${DAWN_ROOT}/build")
    set(GLFW_ROOT "${CMAKE_SOURCE_DIR}/extern/glfw")
    
    # Add GLFW subdirectory and configure it
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    add_subdirectory(${GLFW_ROOT} glfw EXCLUDE_FROM_ALL)
    
    # Include Dawn headers and GLFW
    target_include_directories(rs_engine_webgpu PUBLIC
        ${DAWN_ROOT}/include
        ${DAWN_ROOT}/src
        ${DAWN_BUILD}/gen/include
        ${GLFW_ROOT}/include
    )
    
    # Find and link all required Dawn and dependency libraries
    file(GLOB DAWN_LIBS
        "${DAWN_BUILD}/src/dawn/native/libdawn_native.a"
        "${DAWN_BUILD}/src/dawn/libdawn_proc.a"
        "${DAWN_BUILD}/src/dawn/common/libdawn_common.a"
        "${DAWN_BUILD}/src/dawn/platform/libdawn_platform.a"
        "${DAWN_BUILD}/src/dawn/wire/libdawn_wire.a"
        "${DAWN_BUILD}/src/dawn/glfw/libdawn_glfw.a"
    )

    file(GLOB ABSEIL_LIBS "${DAWN_BUILD}/third_party/abseil/absl/*/libabsl_*.a")
    file(GLOB TINT_LIBS "${DAWN_BUILD}/src/tint/libtint*.a")

    target_link_libraries(rs_engine_webgpu PUBLIC
        ${DAWN_LIBS}
        ${ABSEIL_LIBS}
        ${TINT_LIBS}
        glfw  # Use our submodule GLFW instead of Dawn's
    )
    
    # Add macOS frameworks
    if(APPLE)
        target_link_libraries(rs_engine_webgpu PUBLIC
            "-framework Cocoa"
            "-framework IOKit" 
            "-framework IOSurface"
            "-framework CoreVideo"
            "-framework QuartzCore"
            "-framework Metal"
            "-framework CoreFoundation"
            "-framework Foundation"
        )
    elseif(WIN32)
        target_link_libraries(rs_engine_webgpu PUBLIC
            d3d12 
            dxgi 
            dxguid
        )
    elseif(UNIX)
        target_link_libraries(rs_engine_webgpu PUBLIC
            X11
        )
    endif()
endif()

# Include directories for users of this library
target_include_directories(rs_engine_webgpu PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)
