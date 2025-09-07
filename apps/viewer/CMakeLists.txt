# apps/viewer/CMakeLists.txt

# Check if we're building for web or native
option(BUILD_WEB "Build for web using Emscripten" OFF)

if(BUILD_WEB)
    # Web version using Emscripten
    add_executable(viewer_web main_web.cpp)
    
    # Set C++17 for compatibility
    set_target_properties(viewer_web PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    
    # Emscripten-specific settings
    target_compile_options(viewer_web PRIVATE 
        -sUSE_WEBGPU=1
    )
    
    target_link_options(viewer_web PRIVATE
        -sUSE_WEBGPU=1
        -sASYNCIFY=1
        -sALLOW_MEMORY_GROWTH=1
        --bind
        -sTOTAL_MEMORY=64MB
        -sEXPORTED_RUNTIME_METHODS=["ccall","cwrap"]
        --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/index.html
        -lhtml5
    )
    
    # Copy HTML file to build directory
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/index.html
        ${CMAKE_CURRENT_BINARY_DIR}/index.html
        COPYONLY
    )
    
else()
    # Native version using Dawn WebGPU
    add_executable(viewer_native main_native.cpp)
    
    # Set C++17 for compatibility  
    set_target_properties(viewer_native PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    
    # Use Dawn WebGPU
    set(DAWN_ROOT "${CMAKE_SOURCE_DIR}/extern/dawn")
    set(DAWN_BUILD "${DAWN_ROOT}/build")
    
    # Include Dawn headers
    target_include_directories(viewer_native PRIVATE 
        ${DAWN_ROOT}/include
        ${DAWN_ROOT}/src
        ${DAWN_BUILD}/gen/include
        ${DAWN_ROOT}/third_party/glfw/include
    )
    
    target_link_libraries(viewer_native PRIVATE 
        # Core Dawn libraries
        ${DAWN_BUILD}/src/dawn/native/libdawn_native.a
        ${DAWN_BUILD}/src/dawn/libdawn_proc.a
        ${DAWN_BUILD}/src/dawn/common/libdawn_common.a
        ${DAWN_BUILD}/src/dawn/libdawn_platform.a
        ${DAWN_BUILD}/src/dawn/libdawn_wire.a
        
        # Core Tint libraries
        ${DAWN_BUILD}/src/tint/libtint_api.a
        ${DAWN_BUILD}/src/tint/libtint_lang_core_ir.a
        ${DAWN_BUILD}/src/tint/libtint_lang_core_ir_type.a
        ${DAWN_BUILD}/src/tint/libtint_lang_core_ir_transform.a
        ${DAWN_BUILD}/src/tint/libtint_lang_core_ir_analysis.a
        ${DAWN_BUILD}/src/tint/libtint_lang_core_ir_binary.a
        ${DAWN_BUILD}/src/tint/libtint_lang_wgsl.a
        ${DAWN_BUILD}/src/tint/libtint_lang_wgsl_ast.a
        ${DAWN_BUILD}/src/tint/libtint_lang_wgsl_sem.a
        ${DAWN_BUILD}/src/tint/libtint_lang_wgsl_resolver.a
        ${DAWN_BUILD}/src/tint/libtint_lang_wgsl_inspector.a
        ${DAWN_BUILD}/src/tint/libtint_lang_wgsl_reader.a
        ${DAWN_BUILD}/src/tint/libtint_lang_wgsl_writer.a
        ${DAWN_BUILD}/src/tint/libtint_lang_msl_writer.a
        
        # Essential Abseil libraries  
        ${DAWN_BUILD}/third_party/abseil/absl/container/libabsl_raw_hash_set.a
        ${DAWN_BUILD}/third_party/abseil/absl/strings/libabsl_strings.a
        ${DAWN_BUILD}/third_party/abseil/absl/strings/libabsl_str_format_internal.a
        ${DAWN_BUILD}/third_party/abseil/absl/hash/libabsl_hash.a
        ${DAWN_BUILD}/third_party/abseil/absl/hash/libabsl_city.a
        ${DAWN_BUILD}/third_party/abseil/absl/container/libabsl_hashtablez_sampler.a
        ${DAWN_BUILD}/third_party/abseil/absl/base/libabsl_base.a
        ${DAWN_BUILD}/third_party/abseil/absl/base/libabsl_raw_logging_internal.a
        ${DAWN_BUILD}/third_party/abseil/absl/base/libabsl_malloc_internal.a
        ${DAWN_BUILD}/third_party/abseil/absl/base/libabsl_throw_delegate.a
        ${DAWN_BUILD}/third_party/abseil/absl/container/libabsl_container_common.a
        ${DAWN_BUILD}/third_party/abseil/absl/debugging/libabsl_symbolize.a
        ${DAWN_BUILD}/third_party/abseil/absl/debugging/libabsl_stacktrace.a
        ${DAWN_BUILD}/third_party/abseil/absl/synchronization/libabsl_synchronization.a
        ${DAWN_BUILD}/third_party/abseil/absl/synchronization/libabsl_graphcycles_internal.a
        ${DAWN_BUILD}/third_party/abseil/absl/time/libabsl_time.a
        ${DAWN_BUILD}/third_party/abseil/absl/types/libabsl_optional.a
        
        # Platform libraries
        ${DAWN_BUILD}/third_party/glfw/src/libglfw3.a
    )
    
    # Add macOS frameworks
    if(APPLE)
        target_link_libraries(viewer_native PRIVATE 
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
        target_link_libraries(viewer_native PRIVATE 
            d3d12 
            dxgi 
            dxguid
        )
    elseif(UNIX)
        target_link_libraries(viewer_native PRIVATE 
            X11
        )
    endif()
    
    # Also build a simple fallback version
    add_executable(viewer_simple main.cpp)
    target_link_libraries(viewer_simple PRIVATE rs_engine_webgpu)
    
    set_target_properties(viewer_simple PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    
    target_include_directories(viewer_simple PRIVATE 
        ${DAWN_ROOT}/third_party/glfw/include
    )
    
    target_link_libraries(viewer_simple PRIVATE 
        ${DAWN_BUILD}/third_party/glfw/src/libglfw3.a
    )
    
    if(APPLE)
        target_link_libraries(viewer_simple PRIVATE 
            "-framework Cocoa"
            "-framework IOKit" 
            "-framework CoreVideo"
            "-framework QuartzCore"
            "-framework Metal"
        )
    endif()
endif()
