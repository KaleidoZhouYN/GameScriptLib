#ifndef NCNN_INFER_H
#define NCNN_INFER_H

#include "common.h"
#include "ncnn/net.h"
#include "ncnn/layer.h"

#include<string>
#include<cfloat>
#include<cstdio>
#include<filesystem>

class NCNN_INFER
{
public:
	NCNN_INFER() : _net(nullptr) {}
	virtual ~NCNN_INFER();
	NCNN_INFER(const NCNN_INFER&) = delete;
	virtual void infer(const cv::Mat&);

protected:
	ncnn::Net* _net;  
};

// load model by config from config file
void NCNN_INFER::load(const char* config_file)
{
	// read config file
	read_json(config_file, pt);

	// whether use gpu
	bool useGPU = pt.get<std::string>("useGPU") > "0";
	bool hasGPU; 

#if NCNN_VULKAN
	hasGPU = ncnn::get_gpu_count() > 0;
#else
	hasGPU = 0; 
#endif

	UseGPU = hasGPU && useGPU; 

	// fp16 or int8
	bool useFp16 = pt.get<std::string>("fp16") > "0";

	// model
	std::string param_file = pt.get<std::string>("param_file");
	std::string bin_file = pt.get<std::string>("bin_file");
	
	if (_net)
		delete _net; 
	_net = new ncnn::Net(); 

	// opt 需要在加载前设置
	_net->opt.use_vulkan_compute = UseGPU; 
	_net->opt.use_fp16_arithmetic = useFp16; 
	_net->load_param(param_file.c_str());
	_net->load_model(bin_file.c_str());
}

NCNN_INFER::~NCNN_INFER()
{
	if (_net)
		delete _net; 
}

void NCNN_INFER::infer()
{
	int img_w = bgr.cols; 
	int img_h = bgr.rows; 

	// input ncnn mat
	ncnn::Mat in = ncnn::Mat::from_pixels_resize(bgr.data, 0, img_w, img_h, input_w, input_h);
	const float norm_vals[3] = { 1 / 255.f, 1 / 255.f, 1 / 255.f };
	in.substract_mean_normalize(0, norm_vals);

	ncnn::Extractor ex = Net->create_extractor(); 

	ex.input("images", in_pad);
}
#endif