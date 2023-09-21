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

#include<string>
#include<cfloat>
#include<cstdio>
#include<filesystem>

struct Layer_Info
{
	std::string name; 
};

class NCNN_INFER
{
public:
	NCNN_INFER() : _net(nullptr) {}
	virtual ~NCNN_INFER();
	NCNN_INFER(const NCNN_INFER&) = delete;
	virtual void load(const char*);
	virtual void infer(const cv::Mat&);
	virtual void get_result();

protected:
	ncnn::Net* _net;
	JSON _json; 

	ncnn::Mat _out; 
	std::vector<Layer_Info> out_layers;
};

#endif