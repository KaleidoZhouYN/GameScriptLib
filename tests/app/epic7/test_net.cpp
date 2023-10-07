#include<iostream>
#include<vector>
#include <opencv2/opencv.hpp>
#include <windows.h>
#include "onnx_infer.h"
#include "ncnn_infer.h"

void test_onnx(cv::Mat& croppedImage)
{
	OnnxInfer onnx_infer;
	onnx_infer.load(R"(C:\Users\zhouy\source\repos\GameScriptLib\src\app\epic7\assert\hero_avatar.onnx)");

	cv::Mat image; 
	croppedImage.convertTo(image, CV_32FC3);
	image /= 255.0f;
	// [H,W,C] -> [C,H,W]
	std::vector<cv::Mat> channels(3);
	cv::split(image, channels);

	cv::Mat chw(image.rows * image.cols * 3, 1, CV_32F);
	for (int i = 0; i < 3; ++i) {
		channels[i].reshape(1, image.rows * image.cols).copyTo(chw.rowRange(i * image.rows * image.cols, (i + 1) * image.rows * image.cols));
	}

	float* p = chw.ptr<float>(0);

	std::vector<Ort::Value> inputs;
	inputs.emplace_back(pt_to_tensor<float>(p, onnx_infer.get_input_shapes()[0]));
	//onnx_infer.set_inputs(inputs);

	std::vector<std::string> output_name_ = { "57" };
	std::vector<std::vector<int64_t>> output_shape_ = { {1, 196} };
	onnx_infer.set_outputs(output_name_, output_shape_);
	onnx_infer.forward(inputs);
	auto* results_tensors = onnx_infer.get_result(output_name_[0]);

	float max = -1000;
	int max_index = -1;

	float* results_ = const_cast<Ort::Value*>(results_tensors)->GetTensorMutableData<float>();
	for (int i = 0; i < 196; i++)
		if (max < results_[i])
		{
			max = results_[i];
			max_index = i;
		}
	std::cout << max_index << std::endl;
}

void test_ncnn(cv::Mat& bgr)
{
	std::shared_ptr<NCNN_INFER> ncnn_infer = std::make_shared<NCNN_INFER>();
	ncnn_infer->load(R"(C:\Users\zhouy\source\repos\GameScriptLib\src\app\epic7\assert\hero_avatar_feature.json)");
	ncnn_infer->infer(bgr);
	ncnn_infer->get_result(); 
}

int main() {
	// 初始化 Onnx Runtime

	// 加载图像并转为浮点数
	cv::Mat image = cv::imread(R"(C:\Users\zhouy\source\repos\GameScriptLib\src\app\epic7\resources\photo\equipment\暗扇.jpg)");
	cv::resize(image, image, cv::Size(1280, 800));

	cv::Rect rect(1140, 272, 140, 90);
	cv::Mat croppedImage = image(rect).clone();

	test_ncnn(croppedImage); 
	return 0; 
}