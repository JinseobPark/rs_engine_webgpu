# apps/viewer/CMakeLists.txt
add_executable(mintviewer main.cpp)
target_link_libraries(mintviewer PRIVATE mintengine)

if(NOT BUILD_WEB)
  find_package(glfw3 REQUIRED)
  target_link_libraries(mintviewer PRIVATE glfw webgpu_dawn dawn_native dawncpp)
endif()
