cmake_minimum_required (VERSION 3.13)

link_directories("C:/code/3rd_lib/Blackbone-master/src/out/build/x64-Debug/BlackBone")
link_directories("C:/code/3rd_lib/Blackbone-master/src/3rd_party/DIA/lib/amd64")
add_executable(test_inject test_inject.cpp ${PROJECT_SOURCE_DIR}/src/win/win.cpp ${PROJECT_SOURCE_DIR}/src/inject/injector.cpp)
target_include_directories(test_inject PRIVATE ${PROJECT_SOURCE_DIR}/include/project/win ${PROJECT_SOURCE_DIR}/include/project/inject)
target_include_directories(test_inject PRIVATE ${PROJECT_SOURCE_DIR}/include/project/screenshot)
target_include_directories(test_inject PRIVATE "C:/code/3rd_lib/Blackbone-master/src")

set(OpenCV_DIR "C:/code/3rd_lib/opencv-4.7.0/out/install/x64-Debug")
find_package(OpenCV REQUIRED)
target_include_directories(test_inject PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(test_inject PRIVATE ${OpenCV_LIBS})


target_link_libraries(test_inject PRIVATE BlackBone diaguids)