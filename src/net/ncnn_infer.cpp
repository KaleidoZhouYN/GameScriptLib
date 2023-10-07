#include "ncnn_infer.h"

#include<iostream>

// 建议弄成配置的注册机制
void DefaultPreProcess::operator()(LPARAM src, LPARAM dst)
{
	cv::Mat& bgr = *(reinterpret_cast<cv::Mat*>(src));
	int img_w = bgr.cols;
	int img_h = bgr.rows;

	// input ncnn mat
	ncnn::Mat& in = *(reinterpret_cast<ncnn::Mat*>(dst));
	in = ncnn::Mat::from_pixels_resize(bgr.data, ncnn::Mat::PIXEL_BGR, img_w, img_h, img_w, img_h);
	const float norm_vals[3] = { 1 / 255.f, 1 / 255.f, 1 / 255.f };
	in.substract_mean_normalize(0, norm_vals);
}

void DefaultPostProcess::operator()(LPARAM src, LPARAM dst)
{
}

std::unordered_map<std::string, std::shared_ptr<Process>> NCNN_INFER::PreProcess_Register_Map = { {"default",std::make_shared<DefaultPreProcess>()}};
std::unordered_map<std::string, std::shared_ptr<Process>> NCNN_INFER::PostProcess_Register_Map = { {"default",std::make_shared<DefaultPostProcess>()} };


// load model by config from config file
void NCNN_INFER::load(const char* config_file)
{
	// read config file
	_json.load(config_file); 

	// whether use gpu
	bool useGPU = _json["useGPU"].get<bool>();
	bool hasGPU; 

#if NCNN_VULKAN
	hasGPU = ncnn::get_gpu_count() > 0;
#else
	hasGPU = 0; 
#endif

	useGPU = hasGPU && useGPU; 

	// fp16 or int8
	bool useFp16 = _json["fp16"].get<bool>();

	// model
	std::string param_file = _json["param_file"].get<std::string>();
	std::string bin_file = _json["bin_file"].get<std::string>();

	// output layers
	_input_name = _json["input"].get<std::string>();
	auto layers = _json["out_layers"];
	for (auto layer : layers) {
		out_layers.push_back(Layer_Info(layer["name"].get<std::string>()));
	}
	
	// process
	std::string pre_class = _json["pre_process"]["classname"];
	_pre_process = PreProcess_Register_Map[pre_class]();
	_pre_process->LoadConfig();

	std::string post_class = _json["post_process"]["classname"];
	_post_process = PostProcess_Register_Map[post_class]();
	_post_process->LoadConfig();


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
	// 使用函数指针做预处理
	_pre_process(bgr, _in);

	ncnn::Extractor ex = _net->create_extractor(); 

	ex.input(_input_name.c_str(), _in);
	ex.extract(out_layers[0].name.c_str(), _out);
}

// 替换为函数指针
void NCNN_INFER::get_result()
{
	float* ptr = _out.channel(0);
	std::cout << ptr[0] << std::endl; 
	_post_process(_out, reinterpret_cast<LPARAM>(nullptr));
}
