# apps/fluid_demo/CMakeLists.txt

# Check if we're building for web or native
option(BUILD_WEB "Build for web using Emscripten" OFF)

if(EMSCRIPTEN)
    # Web version using our engine
    add_executable(fluid_demo main.cpp FluidDemoApp.cpp)
    
    # Set C++17 for compatibility
    set_target_properties(fluid_demo PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        SUFFIX ".html"
    )
    
    # Link with our engine library
    target_link_libraries(fluid_demo PRIVATE rs_engine_webgpu)
    
    # Include directories
    target_include_directories(fluid_demo PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}
    )
    
    target_link_options(fluid_demo PRIVATE
        -sUSE_WEBGPU=1
        -sASYNCIFY
        -sALLOW_MEMORY_GROWTH=1
        # 디버깅 옵션들
        -g                              # 디버깅 정보 생성
        -gsource-map                    # 소스 맵 생성
        --emit-symbol-map               # 심볼 맵 생성
        -O0                             # 최적화 레벨 0 (디버깅용)
        --source-map-base=http://localhost:3377/
    )
    
    # Copy HTML file to build directory (always copy, even if newer)
    add_custom_command(TARGET fluid_demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/index.html
            ${CMAKE_CURRENT_BINARY_DIR}/index.html
        COMMENT "Copying updated index.html to build directory"
    )
    
else()
    # Native version using our engine
    add_executable(fluid_demo main.cpp FluidDemoApp.cpp)

    # Set C++17 for compatibility
    set_target_properties(fluid_demo PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )

    # Link with our engine library
    target_link_libraries(fluid_demo PRIVATE rs_engine_webgpu)

    # Include directories
    target_include_directories(fluid_demo PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}
    )

endif()