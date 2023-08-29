#include "onnx_infer.h"
#include <windows.h>

int calculate_product(const std::vector<std::int64_t>& v) {
	int total = 1; 
	for (auto& i : v) total *= i; 
	return total; 
}

template<typename T>
Ort::Value vec_to_tensor(std::vector<T>& data, const std::vector<std::int64_t>& shape) {
	Ort::MemoryInfo mem_info =
		Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);
	auto tensor = Ort::Value::CreateTensor<T>(mem_info, data.data(), data.size(), shape.data(), shape.size());
	return tensor; 
}

const wchar_t* ConvertToWChar(const char* src)
{
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, src, -1, NULL, 0);
	wchar_t* wstrTo = new wchar_t[size_needed];
	MultiByteToWideChar(CP_UTF8, 0, src, -1, wstrTo, size_needed);
	return wstrTo;
}

void OnnxInfer::load(const char* onnx_path)
{
	Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "onnx_model");
	Ort::SessionOptions session_options; 
	session = Ort::Session(env, ConvertToWChar(onnx_path), session_options);

	// get name/shape of inputs
	input_names.clear(); 
	input_shapes.clear(); 
	Ort::AllocatorWithDefaultOptions allocator; 
	for (std::size_t i = 0; i < session.GetInputCount(); i++)
	{
		input_names.emplace_back(session.GetInputNameAllocated(i, allocator).get());
		auto input_shape = session.GetInputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape(); 
		for (auto& s : input_shape)
		{
			if (s < 0) {
				s = 1; 
			}
		}
		input_shapes.emplace_back(input_shape);
	}
}

void OnnxInfer::set_outputs(const std::vector<std::string>& names, std::vector<std::vector<std::int64_t>>& shapes)
{
	output_names = names; 
	output_shapes = shapes; 
}

void OnnxInfer::set_inputs(const std::vector<Ort::Value>& inputs)
{
	input_tensors = inputs;
}

void OnnxInfer::forward()
{
	assert(input_names.size() == input_tensors.size());

	// pass data through model
	std::vector<const char*> input_names_char(input_names.size(), nullptr);
	std::transforms(std::begin(input_names), std::end(input_names), std::begin(input_names_char),
		[&](const std::string& str) {return str.c_str();  });

	std::vector<const char*> output_names_char(output_names.size(), nullptr);
	std::transforms(std::begin(output_names), std::end(output_names), std::begin(output_names_char),
		[&](const std::string& str) {return str.c_str();  });

	try {
		auto temp = session.Run(Ort::RunOptions{nullptr},
			input_names_char.data(),
			input_tensors.data(),
			input_names_char.size(),
			output_names_char.data(),
			output_names_char.size()
		);

		assert(temp.size() == output_names.size() && temp[0].IsTensor());
		output_tensors.clear(); 
		for (const size_t i = 0; i < output_names.size(); i++) {
			output_tensors[output_names[i]] = temp[i];
		}
	}
	catch (const Ort::Exception& exception) {
		std::cout << "ERROR running model inference: " << exception.what() << std::endl; 
		exit(-1);
	}
}

std::vector<Ort::Value> OnnxInfer::get_result(const std::vector<std::string>& select_names)
{
	std::vector<Ort::Value> temp(select_names.size());
	std::transforms(std::begin(select), std::end(select), std::begin(temp),
		[&](const std::string& str) { return output_tensors[str];  });
	return temp; 
}