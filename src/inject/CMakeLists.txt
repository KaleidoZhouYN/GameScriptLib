cmake_minimum_required(VERSION 3.13)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DERECTORY ${CMAKE_BINARY_DIR}/bin)

set(BOOST_ROOT "C:/code/3rd_lib/boost_1_83_0/")
#find_package(Boost REQUIRED COMPONENTS log log_setup filesystem system thread date_time)
#find_package(Boost REQUIRED)
#link_directories(${Boost_LIBRARY_DIRS})

set(SOURCE_FILES "OpenGLScreenShot.cpp" 
${PROJECT_SOURCE_DIR}/src/screenshot/screenshot.cpp
"C:/code/3rd_lib/easyloggingpp-9.96.4/src/easylogging++.cc")
# ´´½¨DLL
link_directories("C:/code/3rd_lib/Detours-4.0.1/lib.X64")
add_library(OpenGLScreenShotDLL SHARED  ${SOURCE_FILES})
target_include_directories(OpenGLScreenShotDLL PRIVATE "C:/code/3rd_lib/Detours-4.0.1/include")
target_include_directories(OpenGLScreenShotDLL PRIVATE "C:/code/3rd_lib/easyloggingpp-9.96.4/src")
target_include_directories(OpenGLScreenShotDLL PRIVATE ${PROJECT_SOURCE_DIR}/include/project/inject)
target_include_directories(OpenGLScreenShotDLL PRIVATE ${PROJECT_SOURCE_DIR}/include/project/screenshot)
target_include_directories(OpenGLScreenShotDLL PRIVATE ${PROJECT_SOURCE_DIR}/include/project/win)
#target_include_directories(OpenGLScreenShotDLL PRIVATE "C:/code/3rd_lib/boost_1_83_0")

target_link_libraries(OpenGLScreenShotDLL PRIVATE OpenGL32 detours)
