include_directories(  ${PROJECT_SOURCE_DIR}/include/project/net)
include_directories(  "C:/code/3rd_lib/onnxruntime-win-x64-1.15.1/include")
include_directories(  ${PROJECT_SOURCE_DIR}/include/project/control)
include_directories(  ${PROJECT_SOURCE_DIR}/include/project/win)
link_directories("C:/code/3rd_lib/Blackbone-master/src/out/build/x64-Debug/BlackBone")
link_directories("C:/code/3rd_lib/Blackbone-master/src/3rd_party/DIA/lib/amd64")
include_directories(  ${PROJECT_SOURCE_DIR}/include/project/control)
include_directories(  ${PROJECT_SOURCE_DIR}/include/project/win)
include_directories(  ${PROJECT_SOURCE_DIR}/include/project/inject)
include_directories(  ${PROJECT_SOURCE_DIR}/include/project/screenshot)
include_directories(  "C:/code/3rd_lib/easyloggingpp-9.96.4/src")
include_directories("C:/code/3rd_lib/Blackbone-master/src")

set(OpenCV_DIR "C:/code/3rd_lib/opencv-4.7.0/out/install/x64-Debug")
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

#add_executable(test_net test_net.cpp ${PROJECT_SOURCE_DIR}/src/net/onnx_infer.cpp )

#target_link_libraries(test_net PRIVATE ${OpenCV_LIBS})

#target_link_libraries(test_net PRIVATE "C:\\code\\3rd_lib\\onnxruntime-win-x64-1.15.1\\lib\\onnxruntime.lib"
#"C:\\code\\3rd_lib\\onnxruntime-win-x64-1.15.1\\lib\\onnxruntime_providers_shared.lib")

#add_executable(test_controller 
#test_controller.cpp
#${PROJECT_SOURCE_DIR}/src/win/win.cpp
#)

#add_executable(test_record 
#test_record.cpp
#${PROJECT_SOURCE_DIR}/src/win/win.cpp
#${PROJECT_SOURCE_DIR}/src/inject/injector.cpp
#${PROJECT_SOURCE_DIR}/src/screenshot/capture_gl.cpp
#${PROJECT_SOURCE_DIR}/src/screenshot/screenshot.cpp
#"C:/code/3rd_lib/easyloggingpp-9.96.4/src/easylogging++.cc")

#target_link_libraries(test_record PRIVATE ${OpenCV_LIBS})
#target_link_libraries(test_record PRIVATE BlackBone diaguids)

add_executable(test_all WIN32
test_all.cpp
${PROJECT_SOURCE_DIR}/src/win/win.cpp
${PROJECT_SOURCE_DIR}/src/inject/injector.cpp
${PROJECT_SOURCE_DIR}/src/screenshot/capture_gl.cpp
${PROJECT_SOURCE_DIR}/src/screenshot/screenshot.cpp
${PROJECT_SOURCE_DIR}/src/net/onnx_infer.cpp
"C:/code/3rd_lib/easyloggingpp-9.96.4/src/easylogging++.cc")

target_link_libraries(test_all PRIVATE ${OpenCV_LIBS})
target_link_libraries(test_all PRIVATE BlackBone diaguids)
target_link_libraries(test_all PRIVATE "C:\\code\\3rd_lib\\onnxruntime-win-x64-1.15.1\\lib\\onnxruntime.lib"
"C:\\code\\3rd_lib\\onnxruntime-win-x64-1.15.1\\lib\\onnxruntime_providers_shared.lib")

