cmake_minimum_required(VERSION 3.8)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DERECTORY ${CMAKE_BINARY_DIR}/bin)

set(SOURCE_FILES "OpenGLScreenShot.cpp" 
${PROJECT_SOURCE_DIR}/src/screenshot/screenshot.cpp)
# ´´½¨DLL
link_directories("C:/code/3rd_lib/Detours-4.0.1/lib.X64")
add_library(OpenGLScreenShotDLL SHARED  ${SOURCE_FILES})
target_include_directories(OpenGLScreenShotDLL PRIVATE "C:/code/3rd_lib/Detours-4.0.1/include")
target_include_directories(OpenGLScreenShotDLL PRIVATE ${PROJECT_SOURCE_DIR}/include/project/inject)
target_include_directories(OpenGLScreenShotDLL PRIVATE ${PROJECT_SOURCE_DIR}/include/project/screenshot)
target_include_directories(OpenGLScreenShotDLL PRIVATE ${PROJECT_SOURCE_DIR}/include/project/win)

target_link_libraries(OpenGLScreenShotDLL OpenGL32 detours)
