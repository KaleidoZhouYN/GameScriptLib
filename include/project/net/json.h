#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class JSON
{
public:
	JSON()
	{

	}
	~JSON() {}
	void load(const char* config_file)
	{
		std::ifstream f(config_file);
		data = json::parse(f);
	}
	template<typename T>
	json& operator [](const std::string& key)
	{
		return data[key];
	}
	const json& operator[](const std::string& key) const
	{
		return data.at(key);
	}
private:
	json data; 
};