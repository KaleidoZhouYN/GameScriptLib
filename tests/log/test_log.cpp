#include "easylogging++.h"
#define ELPP_THREAD_SAFE

INITIALIZE_EASYLOGGINGPP

int main()
{
	LOG(INFO) << "aaa";
	return 0; 
}