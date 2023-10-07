#ifndef _PROCESS_H
#define _PROCESS_H

class Process
{
	PreProcess() {}
	virtual ~PreProcess() {}
	virtual void LoadConfig() = 0;
	virtual void operator()(LPARAM src, LPARAM dst) = 0;
};

#endif