add_executable(test_net test_net.cpp ${PROJECT_SOURCE_DIR}/src/net/onnx_infer.cpp )

set(OpenCV_DIR "C:/code/3rd_lib/opencv-4.7.0/out/install/x64-Debug")
find_package(OpenCV REQUIRED)
target_include_directories(test_net PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(test_net PRIVATE ${OpenCV_LIBS})

target_include_directories(test_net PRIVATE ${PROJECT_SOURCE_DIR}/include/project/net)
target_include_directories(test_net PRIVATE "C:/code/3rd_lib/onnxruntime-win-x64-1.15.1/include")
target_link_libraries(test_net PRIVATE "C:\\code\\3rd_lib\\onnxruntime-win-x64-1.15.1\\lib\\onnxruntime.lib"
"C:\\code\\3rd_lib\\onnxruntime-win-x64-1.15.1\\lib\\onnxruntime_providers_shared.lib")
