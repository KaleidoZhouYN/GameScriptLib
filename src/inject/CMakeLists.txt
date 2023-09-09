cmake_minimum_required(VERSION 3.13)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DERECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories("C:/code/3rd_lib/Detours-4.0.1/include")
include_directories("C:/code/3rd_lib/easyloggingpp-9.96.4/src")
include_directories(${PROJECT_SOURCE_DIR}/include/project/inject)
include_directories(${PROJECT_SOURCE_DIR}/include/project/screenshot)
include_directories(${PROJECT_SOURCE_DIR}/include/project/win)

# ´´½¨DLL
link_directories("C:/code/3rd_lib/Detours-4.0.1/lib.X64")

add_library(OpenGLScreenShotDLL SHARED  "OpenGLScreenShot.cpp" 
"C:/code/3rd_lib/easyloggingpp-9.96.4/src/easylogging++.cc")

target_link_libraries(OpenGLScreenShotDLL PRIVATE OpenGL32 detours)

add_library(D3D11ScreenShotDLL SHARED  "DirectX11.cpp"
"C:/code/3rd_lib/easyloggingpp-9.96.4/src/easylogging++.cc" )

target_link_libraries(D3D11ScreenShotDLL PRIVATE detours)