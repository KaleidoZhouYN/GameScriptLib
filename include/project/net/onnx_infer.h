#ifndef ONNX_INFER_H
#define ONNX_INFER_H
#include <cstdint>
#include <algorithm>
#include <cassert>
#include <cstddef>
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
	OnnxInfer() {};
	~OnnxInfer() {
		delete session; 
	};

	void load(const char* onnx_path);
	//void set_inputs(const std::vector<Ort::Value>&);
	void set_outputs(const std::vector<std::string>&, std::vector<std::vector<std::int64_t>>&);
	const std::vector<std::vector<std::int64_t>> get_input_shapes();

	void forward(const std::vector<Ort::Value>&);
	const Ort::Value* get_result(const std::string& output_name);
	const std::vector<Ort::Value*> get_result(const std::vector<std::string>& output_name);
private:
	Ort::Session* session = nullptr;
	std::vector<std::string> input_names = {};
	std::vector<std::vector<std::int64_t>> input_shapes = {};
	std::vector<std::string> output_names = {};
	std::vector<std::vector<std::int64_t>> output_shapes = {};

	//std::vector<Ort::Value> input_tensors = {};

	// we try to make output_tensors a std::map<std::string, Ort::Value> but failed
	// becuase while std::map record a element, it must constructed the Ort::Value first
	// but Ort::Value didn't have a default constructor
	// maybe need to use Ort::Value& or std::shared_ptr<Ort::Value>
	std::vector<Ort::Value> output_tensors = {};

	// could not use shared_ptr because cannot use stared_ptr(new Qrt::Value())
	std::map<std::string, Ort::Value*> output_map; 
};

int calculate_product(const std::vector<std::int64_t>&);

template<typename T>
Ort::Value vec_to_tensor(std::vector<T>& data, const std::vector<std::int64_t>& shape) {
	Ort::MemoryInfo mem_info =
		Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);
	auto tensor = Ort::Value::CreateTensor<T>(mem_info, data.data(), data.size(), shape.data(), shape.size());
	return tensor;
}

template<typename T>
Ort::Value pt_to_tensor(T* p, const std::vector<std::int64_t>& shape)
{
	Ort::MemoryInfo mem_info =
		Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);
	auto tensor = Ort::Value::CreateTensor<T>(mem_info, p, calculate_product(shape), shape.data(), shape.size());
	return tensor;
}

#endif