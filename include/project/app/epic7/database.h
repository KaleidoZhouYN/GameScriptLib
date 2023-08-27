#include <map>
#include <string>

// 2023/08/27: whether to make it a singleton
<template class T>
class Database
{
public:
	Database() = default; 
	virtual ~Database() {} = 0; 

	virtual void push(const std::string& k_, const T& v_);
	virtual void get(const std::string& k_);

protected:
	std::map<std::string, std::string> kv_store = {};
};


class HeroDatabase
{
	HeroDatabase(const std::string& f) : _folder(f), kv_store({}) {}
	virtual ~HeroDatabase() {}

	virtual void push(const std::string& k_, const cv::Mat& v_);
	virtual void get(const std::string& k_); 

private:
	std::map<std::string, std::string> kv_store = {}; 
	std::string _folder; 
};
