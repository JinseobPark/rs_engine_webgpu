# apps/viewer/CMakeLists.txt

# Check if we're building for web or native
option(BUILD_WEB "Build for web using Emscripten" OFF)

if(EMSCRIPTEN)
    # Web version using our engine
    add_executable(viewer main.cpp)
    
    # Set C++17 for compatibility
    set_target_properties(viewer PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        SUFFIX ".html"
    )
    
    # Link with our engine library
    target_link_libraries(viewer PRIVATE rs_engine_webgpu)
    
    # Include directories
    target_include_directories(viewer PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}
    )
    
    target_link_options(viewer PRIVATE
        --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/index.html
    )
    
    # Copy HTML file to build directory
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/index.html
        ${CMAKE_CURRENT_BINARY_DIR}/index.html
        COPYONLY
    )
    
else()
    # Native version using our engine - start simple
    add_executable(viewer main.cpp)
    
    # Set C++17 for compatibility
    set_target_properties(viewer PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    
    # For now, just create a basic version that works
    # TODO: Integrate rs_engine_webgpu when Dawn linking is fixed
    
    # Use Dawn WebGPU directly (copy from working legacy version)
    set(DAWN_ROOT "${CMAKE_SOURCE_DIR}/extern/dawn")
    set(DAWN_BUILD "${DAWN_ROOT}/build")
    
    # Include Dawn headers
    target_include_directories(viewer PRIVATE 
        ${DAWN_ROOT}/include
        ${DAWN_ROOT}/src
        ${DAWN_BUILD}/gen/include
        ${DAWN_ROOT}/third_party/glfw/include
        ${CMAKE_SOURCE_DIR}  # For engine headers
    )
    
    # Use minimal Dawn libraries for now
    target_link_libraries(viewer PRIVATE 
        ${DAWN_BUILD}/src/dawn/native/libdawn_native.a
        ${DAWN_BUILD}/src/dawn/libdawn_proc.a
        ${DAWN_BUILD}/src/dawn/common/libdawn_common.a
        ${DAWN_BUILD}/src/dawn/platform/libdawn_platform.a
        ${DAWN_BUILD}/src/dawn/wire/libdawn_wire.a
        ${DAWN_BUILD}/third_party/glfw/src/libglfw3.a
    )
    
    # Add macOS frameworks
    if(APPLE)
        target_link_libraries(viewer PRIVATE 
            "-framework Cocoa"
            "-framework IOKit" 
            "-framework IOSurface"
            "-framework CoreVideo"
            "-framework QuartzCore"
            "-framework Metal"
            "-framework CoreFoundation"
            "-framework Foundation"
        )
    endif()
endif()
