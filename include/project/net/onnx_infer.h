#include <cstdint>
#include<iostream>
#include<vector>
#include <sstream>
#include <onnxruntime_cxx_api.h>
#include <cpu_provider_factory.h>
#include <map>

/* brief: infer class of onnx model, for model inference, we only consider the forward part, 
* and ignore the pre_processing and pos_processing part
* which we will add in concrete Net_infer model
*/
class OnnxInfer
{
public:
	OnnxInfer();
	~OnnxInfer() {};

	void load(const char* onnx_path);
	void set_inputs(const std::vector<Ort::Value>&);
	void set_outputs(const std::vector<std::string>&, std::vector<std::vector<std::int64_t>>&);
	void forward();
	Ort::Value get_result(const std::string& output_name);
private:
	Ort::Session session;
	std::vector<std::string> input_names = {};
	std::vector<std::vector<std::int64_t>> input_shapes = {};
	std::vector<std::string> output_names = {};
	std::vector<std::vector<std::int64_t>> outptu_shapes = {};

	std::vector<Ort::Value> input_tensors = {};
	std::map<std::string, Ort::Value> output_tensors = {};
};

int calculate_product(const std::vector<std::int64_t>&);

template<typename T>
Ort::Value vec_to_tensor(std::vector<T>&, const std::is_trivially_move_constructible<std::int64_t>&);