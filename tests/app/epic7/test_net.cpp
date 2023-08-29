#include<iostream>
#include<vector>
#include <opencv2/opencv.hpp>
#include <windows.h>
#include "onnx_infer.h"



int main() {
	// 初始化 Onnx Runtime
	OnnxInfer onnx_infer;
	onnx_infer.load(R"(C:\Users\zhouy\source\repos\GameScriptLib\src\app\epic7\tools\hero_avatar.onnx)");

	/*/
	Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "ONNXRuntimeModel");

	Ort::RunOptions run_options;

	const wchar_t* model_path = ConvertToWChar(R"(C:\Users\zhouy\source\repos\GameScriptLib\src\app\epic7\tools\hero_avatar.onnx)");
	//const wchar_t* model_path = ConvertToWChar(R"(mnist.onnx)");
	Ort::Session session_{env, model_path, Ort::SessionOptions{nullptr}};
	*/

	// 加载图像并转为浮点数
	cv::Mat image = cv::imread(R"(C:\Users\zhouy\source\repos\GameScriptLib\src\app\epic7\assert\photo\3\avatar\木龙.jpg)");
	cv::resize(image, image, cv::Size(64, 48));
	cv::imwrite("temp.jpg", image);
	image.convertTo(image, CV_32FC3);
	image /= 255.0f; 

	// [H,W,C] -> [C,H,W]
	std::vector<cv::Mat> channels(3);
	cv::split(image, channels); 

	cv::Mat chw(image.rows * image.cols * 3, 1, CV_32F);
	for (int i = 0; i < 3; ++i) {
		channels[i].reshape(1, image.rows * image.cols).copyTo(chw.rowRange(i * image.rows * image.cols, (i + 1) * image.rows * image.cols));
	}

	float* p = chw.ptr<float>(0); 

	/*
	// 创建一个onnx输入张量
	//std::vector<int64_t> input_shape = { 1, 3, 48, 64 };
	//auto memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
	std::array<float, 3*64* 48> input_image_{};
	std::array<float, 125> results_{};
	std::array<int64_t, 4> input_shape_{1, 3, 48, 64};
	std::array<int64_t, 2> output_shape_{1, 125};

	auto memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
	auto input_tensor_ = Ort::Value::CreateTensor<float>(memory_info, p, input_image_.size(),
		input_shape_.data(), input_shape_.size());
	auto output_tensor_ = Ort::Value::CreateTensor<float>(memory_info, results_.data(), results_.size(),
		output_shape_.data(), output_shape_.size());

	// 推理
	const char* input_names[] = { "input.1" };
	const char* output_names[] = { "21" };

	session_.Run(run_options, input_names, &input_tensor_, 1, output_names, &output_tensor_, 1);
	*/
	std::vector<Ort::Value> inputs;
	inputs.emplace_back(pt_to_tensor<float>(p, onnx_infer.get_input_shapes()[0]));
	//onnx_infer.set_inputs(inputs);

	std::vector<std::string> output_name_ = { "21" };
	std::vector<std::vector<int64_t>> output_shape_ = { {1, 125} };
	onnx_infer.set_outputs(output_name_, output_shape_);
	onnx_infer.forward(inputs); 
	auto* results_tensors = onnx_infer.get_result(output_name_[0]);

	float max = -1000;
	int max_index = -1;
	
	float* results_ = const_cast<Ort::Value*>(results_tensors)->GetTensorMutableData<float>();
	for (int i = 0; i < 125; i++)
		if (max < results_[i])
		{
			max = results_[i];
			max_index = i;
		}
	std::cout << max_index << std::endl;
	
	return 0; 
}