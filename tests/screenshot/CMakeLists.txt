cmake_minimum_required (VERSION 3.8)

include_directories(${PROJECT_SOURCE_DIR}/include/project/screenshot)

add_executable(test_screenshot test_screenshot.cpp ${PROJECT_SOURCE_DIR}/src/screenshot/screenshot.cpp)
target_link_libraries(test_screenshot OpenGL32)