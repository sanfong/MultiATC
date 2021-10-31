#pragma once
#include "AllHeader.h"
#include <mutex>

struct LogMsg
{
	string str;
	int color;
};
class ConsoleLog
{
private:
	XY position;
	int height;
	vector<LogMsg> msgs;
	std::mutex mtx;

public:
	inline ConsoleLog(XY pos = XY(0, 25), int h = 5)
	{
		position = pos;
		height = h;
	}
	inline void print(string strLog, int color = 7)
	{
		if (strLog == "") return; // Error: strLog is null
		mtx.lock();
		msgs.push_back(LogMsg{ strLog, color });
		if (msgs.size() > height)
		{
			msgs.erase(msgs.begin());
		}
		mtx.unlock();
	}
	inline void clear()
	{
		msgs.clear();
	}
	inline void showOn(RenderConsole& console)
	{
		for (size_t i = 0; i < msgs.size(); i++)
		{
			console.draw(msgs[i].str, position + XY(0, i), msgs[i].color);
		}
	}
};

extern ConsoleLog debug;