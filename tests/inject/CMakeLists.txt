cmake_minimum_required (VERSION 3.13)

#set(BOOST_ROOT "C:/code/3rd_lib/boost_1_83_0/")
#find_package(Boost REQUIRED COMPONENTS log log_setup filesystem system thread date_time)
#find_package(Boost REQUIRED)
#link_directories(${Boost_LIBRARY_DIRS})

link_directories("C:/code/3rd_lib/Blackbone-master/src/out/build/x64-Debug/BlackBone")
link_directories("C:/code/3rd_lib/Blackbone-master/src/3rd_party/DIA/lib/amd64")
add_executable(test_inject test_inject.cpp ${PROJECT_SOURCE_DIR}/src/win/win.cpp ${PROJECT_SOURCE_DIR}/src/inject/injector.cpp "C:/code/3rd_lib/easyloggingpp-9.96.4/src/easylogging++.cc")
target_include_directories(test_inject PRIVATE ${PROJECT_SOURCE_DIR}/include/project/win ${PROJECT_SOURCE_DIR}/include/project/inject)
target_include_directories(test_inject PRIVATE ${PROJECT_SOURCE_DIR}/include/project/screenshot)
target_include_directories(test_inject PRIVATE "C:/code/3rd_lib/Blackbone-master/src")
#target_include_directories(test_inject PRIVATE "C:/code/3rd_lib/boost_1_83_0")
target_include_directories(test_inject PRIVATE "C:/code/3rd_lib/easyloggingpp-9.96.4/src")

target_link_libraries(test_inject PRIVATE BlackBone diaguids)

set(OpenCV_DIR "C:/code/3rd_lib/opencv-4.7.0/out/install/x64-Debug")
find_package(OpenCV REQUIRED)
target_include_directories(test_inject PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(test_inject PRIVATE ${OpenCV_LIBS})
