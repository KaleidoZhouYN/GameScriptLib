#include "ncnn_infer.h"

#include<iostream>

// load model by config from config file
void NCNN_INFER::load(const char* config_file)
{
	// read config file
	_json.load(config_file); 

	// whether use gpu
	bool useGPU = _json["useGPU"];
	bool hasGPU; 

#if NCNN_VULKAN
	hasGPU = ncnn::get_gpu_count() > 0;
#else
	hasGPU = 0; 
#endif

	useGPU = hasGPU && useGPU; 

	// fp16 or int8
	bool useFp16 = _json["fp16"];

	// model
	std::string param_file = _json["param_file"];
	std::string bin_file = _json["bin_file"];
	
	if (_net)
		delete _net; 
	_net = new ncnn::Net(); 

	// opt 需要在加载前设置
	_net->opt.use_vulkan_compute = useGPU; 
	_net->opt.use_fp16_arithmetic = useFp16; 
	_net->load_param(param_file.c_str());
	_net->load_model(bin_file.c_str());
}

NCNN_INFER::~NCNN_INFER()
{
	if (_net)
		delete _net; 
}

void NCNN_INFER::infer(const cv::Mat& bgr)
{
	int img_w = bgr.cols; 
	int img_h = bgr.rows; 

	// input ncnn mat
	ncnn::Mat in = ncnn::Mat::from_pixels_resize(bgr.data, 0, img_w, img_h, img_w, img_h);
	const float norm_vals[3] = { 1 / 255.f, 1 / 255.f, 1 / 255.f };
	in.substract_mean_normalize(0, norm_vals);

	ncnn::Extractor ex = _net->create_extractor(); 

	ex.input("images", in);
	ex.extract(out_layers[0].name.c_str(), _out);
}

void NCNN_INFER::get_result()
{
	float* ptr = _out.channel(0);
	std::cout << ptr[0] << std::endl; 
}
