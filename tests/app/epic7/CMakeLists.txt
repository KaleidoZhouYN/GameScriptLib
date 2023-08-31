add_executable(test_net test_net.cpp ${PROJECT_SOURCE_DIR}/src/net/onnx_infer.cpp )

set(OpenCV_DIR "C:/code/3rd_lib/opencv-4.7.0/out/install/x64-Debug")
find_package(OpenCV REQUIRED)
target_include_directories(test_net PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(test_net PRIVATE ${OpenCV_LIBS})

target_include_directories(test_net PRIVATE ${PROJECT_SOURCE_DIR}/include/project/net)
target_include_directories(test_net PRIVATE "C:/code/3rd_lib/onnxruntime-win-x64-1.15.1/include")
target_link_libraries(test_net PRIVATE "C:\\code\\3rd_lib\\onnxruntime-win-x64-1.15.1\\lib\\onnxruntime.lib"
"C:\\code\\3rd_lib\\onnxruntime-win-x64-1.15.1\\lib\\onnxruntime_providers_shared.lib")

add_executable(test_controller 
test_controller.cpp
${PROJECT_SOURCE_DIR}/src/win/win.cpp
)
target_include_directories(test_controller PRIVATE ${PROJECT_SOURCE_DIR}/include/project/control)
target_include_directories(test_controller PRIVATE ${PROJECT_SOURCE_DIR}/include/project/win)

link_directories("C:/code/3rd_lib/Blackbone-master/src/out/build/x64-Debug/BlackBone")
link_directories("C:/code/3rd_lib/Blackbone-master/src/3rd_party/DIA/lib/amd64")

add_executable(test_record 
test_record.cpp
${PROJECT_SOURCE_DIR}/src/win/win.cpp
${PROJECT_SOURCE_DIR}/src/inject/injector.cpp
${PROJECT_SOURCE_DIR}/src/screenshot/capture_gl.cpp
${PROJECT_SOURCE_DIR}/src/screenshot/screenshot.cpp
"C:/code/3rd_lib/easyloggingpp-9.96.4/src/easylogging++.cc")
target_include_directories(test_record PRIVATE ${PROJECT_SOURCE_DIR}/include/project/control)
target_include_directories(test_record PRIVATE ${PROJECT_SOURCE_DIR}/include/project/win)
target_include_directories(test_record PRIVATE ${PROJECT_SOURCE_DIR}/include/project/inject)
target_include_directories(test_record PRIVATE ${PROJECT_SOURCE_DIR}/include/project/screenshot)
target_include_directories(test_record PRIVATE "C:/code/3rd_lib/easyloggingpp-9.96.4/src")

set(OpenCV_DIR "C:/code/3rd_lib/opencv-4.7.0/out/install/x64-Debug")
find_package(OpenCV REQUIRED)
target_include_directories(test_record PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(test_record PRIVATE ${OpenCV_LIBS})

target_include_directories(test_record PRIVATE "C:/code/3rd_lib/Blackbone-master/src")
target_link_libraries(test_record PRIVATE BlackBone diaguids)

