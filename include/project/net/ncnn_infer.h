#ifndef NCNN_INFER_H
#define NCNN_INFER_H

#if defined(USE_NCNN_SIMPLEOCV)
#include "simpleocv.h"
#else
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#endif

#include "json.h"
#include "ncnn/net.h"
#include "ncnn/layer.h"
#include "process.h"

#include<string>
#include<cfloat>
#include<cstdio>
#include<filesystem>

struct Layer_Info
{
	Layer_Info(const std::string name_) {
		name = name_;
	}
	std::string name; 
};

class DefaultPreProcess : public Process {
	DefaultPreProcess():Process() {}
	~DefaultPreProcess() {}
	void LoadConfig() override {};
	void operator()(LPARAM src, LPARAM dst) override;
};

class DefaultPostProcess : public Process {
	DefaultPostProcess() :Process() {}
	~DefaultPostProcess() {}
	void LoadConfig() override {};
	void operator()(LPARAM src, LPARAM dst) override;
};

class NCNN_INFER
{
public:
	static std::unordered_map<std::string, std::shared_ptr<Process>> PreProcess_Register_Map;
	static std::unordered_map<std::string, std::shared_ptr<Process>> PostProcess_Register_Map;
public:
	NCNN_INFER() : _net(nullptr) {
	}
	virtual ~NCNN_INFER();
	NCNN_INFER(const NCNN_INFER&) = delete;
	virtual void load(const char*);
	virtual void infer(const cv::Mat&);
	virtual void get_result();

protected:
	ncnn::Net* _net;
	JSON _json; 
	ncnn::Mat _in,_out; 

	std::string _input_name;
	std::vector<Layer_Info> out_layers;
	std::shared_ptr<Process> _pre_process;
	std::shared_ptr<Process> _post_process;
};


#endif