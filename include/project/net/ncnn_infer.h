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
#include "cus_process.h"

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


class DefaultPreProcess : public BaseProcess {
public:
	DefaultPreProcess() : BaseProcess() {}
	~DefaultPreProcess() {}
	void LoadConfig() override {};
	void doProcess(LPARAM src, LPARAM dst) override;
};

class DefaultPostProcess : public BaseProcess {
public:
	DefaultPostProcess() : BaseProcess() {}
	~DefaultPostProcess() {}
	void LoadConfig() override {};
	void doProcess(LPARAM src, LPARAM dst) override;
};


class NCNN_INFER
{
public:
	typedef std::unordered_map<std::string, BaseProcess* (*)()> map_type;
	static map_type PreProcess_Register_Map;
	static map_type PostProcess_Register_Map;
public:
	NCNN_INFER() : _net(nullptr) {
	}
	virtual ~NCNN_INFER();
	NCNN_INFER(const NCNN_INFER&) = delete;
	virtual void load(const char*);
	virtual void infer(const cv::Mat&);
	virtual void get_result(LPARAM dst);

protected:
	ncnn::Net* _net;
	JSON _json; 
	ncnn::Mat _in,_out; 

	std::string _input_name;
	std::vector<Layer_Info> out_layers;
	BaseProcess* _pre_process;
	BaseProcess* _post_process;
};


#endif