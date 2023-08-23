cmake_minimum_required (VERSION 3.8)

add_executable(test_inject test_inject.cpp ${PROJECT_SOURCE_DIR}/src/win/win.cpp ${PROJECT_SOURCE_DIR}/src/screenshot/screenshot.cpp ${PROJECT_SOURCE_DIR}/src/inject/shared_memory.cpp)
target_include_directories(test_inject PRIVATE ${PROJECT_SOURCE_DIR}/include/project/win ${PROJECT_SOURCE_DIR}/include/project/inject)
target_include_directories(test_inject PRIVATE ${PROJECT_SOURCE_DIR}/include/project/screenshot)

set(OpenCV_DIR "C:/code/3rd_lib/opencv-4.7.0/out/install/x64-Debug")
find_package(OpenCV REQUIRED)
target_include_directories(test_inject PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(test_inject PRIVATE ${OpenCV_LIBS})