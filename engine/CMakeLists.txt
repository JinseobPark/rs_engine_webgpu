# engine/CMakeLists.txt

# Generate embedded shaders for web builds
if(EMSCRIPTEN)
    set(EMBEDDED_SHADERS_HEADER "${CMAKE_CURRENT_BINARY_DIR}/rendering/EmbeddedShaders.h")

    add_custom_command(
        OUTPUT ${EMBEDDED_SHADERS_HEADER}
        COMMAND python3 "${CMAKE_SOURCE_DIR}/scripts/embed_shaders.py"
                "${CMAKE_SOURCE_DIR}/shaders"
                "${EMBEDDED_SHADERS_HEADER}"
        DEPENDS "${CMAKE_SOURCE_DIR}/scripts/embed_shaders.py"
        COMMENT "Generating embedded shaders for web build"
        VERBATIM
    )

    add_custom_target(generate_embedded_shaders
        DEPENDS ${EMBEDDED_SHADERS_HEADER}
    )
endif()

# Add ImGui source files
set(IMGUI_SOURCES
    ${CMAKE_SOURCE_DIR}/extern/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/extern/imgui/imgui_demo.cpp
    ${CMAKE_SOURCE_DIR}/extern/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/extern/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/extern/imgui/imgui_widgets.cpp
)

# Add ImGui backend sources based on platform
if(EMSCRIPTEN)
    # For web builds, don't use the WebGPU backend due to API compatibility issues
    # Use minimal ImGui setup for web
    set(IMGUI_BACKEND_SOURCES
        # No backend sources for web - we'll handle rendering manually
    )
else()
    set(IMGUI_BACKEND_SOURCES
        ${CMAKE_SOURCE_DIR}/extern/imgui/backends/imgui_impl_wgpu.cpp
        ${CMAKE_SOURCE_DIR}/extern/imgui/backends/imgui_impl_glfw.cpp
    )
endif()

# Create the engine library
if(EMSCRIPTEN)
    add_library(rs_engine_webgpu STATIC
        # Core infrastructure
        core/Engine.cpp
        
        # Systems
        systems/application/ApplicationSystem.cpp
        systems/input/InputSystem.cpp
        systems/input/CameraController.cpp
        systems/rendering/RenderSystem.cpp
        systems/physics/PhysicsSystem.cpp
        systems/resource/ResourceSystem.cpp
        
        # Rendering
        rendering/ShaderManager.cpp
        rendering/scene/Camera.cpp
        rendering/scene/Scene.cpp
        rendering/scene/SceneObject.cpp
        
        # GUI
        gui/ImGuiManager.cpp
        
        # Resources
        resource/ResourceManager.cpp
        resource/model/Mesh.cpp
        resource/model/Model.cpp
        resource/texture/Texture.cpp
        
        # ImGui
        ${IMGUI_SOURCES}
        ${IMGUI_BACKEND_SOURCES}
    )

    # Make sure embedded shaders are generated before building
    add_dependencies(rs_engine_webgpu generate_embedded_shaders)

    # Add the directory containing the generated header to include path
    target_include_directories(rs_engine_webgpu PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
else()
    add_library(rs_engine_webgpu STATIC
        # Core infrastructure
        core/Engine.cpp
        
        # Systems
        systems/application/ApplicationSystem.cpp
        systems/input/InputSystem.cpp
        systems/input/CameraController.cpp
        systems/rendering/RenderSystem.cpp
        systems/physics/PhysicsSystem.cpp
        systems/resource/ResourceSystem.cpp
        
        # Rendering
        rendering/ShaderManager.cpp
        rendering/scene/Camera.cpp
        rendering/scene/Scene.cpp
        rendering/scene/SceneObject.cpp
        
        # GUI
        gui/ImGuiManager.cpp
        
        # Resources
        resource/ResourceManager.cpp
        resource/model/Mesh.cpp
        resource/model/Model.cpp
        resource/texture/Texture.cpp
        
        # ImGui
        ${IMGUI_SOURCES}
        ${IMGUI_BACKEND_SOURCES}
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

    # For Emscripten, don't define any ImGui WebGPU backend flags
    # The backend will automatically detect the proper WebGPU implementation
    
    target_link_options(rs_engine_webgpu INTERFACE
        -sUSE_WEBGPU=1
        -sASYNCIFY=1
        -sALLOW_MEMORY_GROWTH=1
        --bind
        -sTOTAL_MEMORY=256MB
        -sEXPORTED_RUNTIME_METHODS=["ccall","cwrap"]
        -lhtml5
        # 디버깅 옵션들
        -g                              # 디버깅 정보 생성
        -gsource-map                    # 소스 맵 생성
        --emit-symbol-map               # 심볼 맵 생성
        -O0                             # 최적화 레벨 0 (디버깅용)
        -sASSERTIONS=1                  # assertion 활성화
        -sSAFE_HEAP=1                   # 메모리 안전성 체크
        -sDISABLE_EXCEPTION_CATCHING=0  # 예외 처리 활성화
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

    # Define GLFW_NO_API for native builds
    # target_compile_definitions(rs_engine_webgpu PUBLIC GLFW_NO_API)

    # Define ImGui WebGPU backend for Dawn
    target_compile_definitions(rs_engine_webgpu PUBLIC IMGUI_IMPL_WEBGPU_BACKEND_DAWN)
    
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
    ${CMAKE_SOURCE_DIR}/extern/imgui
    ${CMAKE_SOURCE_DIR}/extern/imgui/backends
)
