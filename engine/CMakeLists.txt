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
        -sTOTAL_MEMORY=64MB
        -sEXPORTED_RUNTIME_METHODS=["ccall","cwrap"]
        -lhtml5
    )
else()
    # Native version setup using Dawn WebGPU
    set(DAWN_ROOT "${CMAKE_SOURCE_DIR}/extern/dawn")
    set(DAWN_BUILD "${DAWN_ROOT}/build")
    
    # Include Dawn headers
    target_include_directories(rs_engine_webgpu PUBLIC
        ${DAWN_ROOT}/include
        ${DAWN_ROOT}/src
        ${DAWN_BUILD}/gen/include
        ${DAWN_ROOT}/third_party/glfw/include
    )
    
    # Find Dawn libraries dynamically
    find_library(DAWN_NATIVE_LIB dawn_native PATHS ${DAWN_BUILD}/src/dawn/native NO_DEFAULT_PATH)
    find_library(DAWN_PROC_LIB dawn_proc PATHS ${DAWN_BUILD}/src/dawn NO_DEFAULT_PATH)
    find_library(DAWN_COMMON_LIB dawn_common PATHS ${DAWN_BUILD}/src/dawn/common NO_DEFAULT_PATH)
    find_library(DAWN_PLATFORM_LIB dawn_platform PATHS ${DAWN_BUILD}/src/dawn/platform NO_DEFAULT_PATH)
    find_library(DAWN_WIRE_LIB dawn_wire PATHS ${DAWN_BUILD}/src/dawn/wire NO_DEFAULT_PATH)
    find_library(GLFW_LIB glfw3 PATHS ${DAWN_BUILD}/third_party/glfw/src NO_DEFAULT_PATH)
    
    target_link_libraries(rs_engine_webgpu PUBLIC
        ${DAWN_NATIVE_LIB}
        ${DAWN_PROC_LIB}
        ${DAWN_COMMON_LIB}
        ${DAWN_PLATFORM_LIB}
        ${DAWN_WIRE_LIB}
        ${GLFW_LIB}
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
