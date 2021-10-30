#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <Windows.h>
#include <time.h>
#include <string.h>

using namespace std;

class AsciiText
{
private:
	ifstream txtFile;

public:
	vector<string> texts;
	bool transparent_whitespace;

	inline AsciiText() { transparent_whitespace = false; }
	inline AsciiText(const char* filename)
	{
		transparent_whitespace = false;
		txtFile.open(filename);
		if (txtFile.fail())
		{
			cout << "\"" << filename << "\"" << " cannot be opened." << endl;
			return;
		}
		string str;
		while (getline(txtFile, str))
		{
			texts.push_back(str);
		}
		txtFile.close();
	}
	inline bool loadFromFile(const char* filename)
	{
		texts.clear();
		txtFile.open(filename);
		if (txtFile.fail())
			return false;

		string str;
		while (getline(txtFile, str))
		{
			texts.push_back(str);
		}
		txtFile.close();
		return true;
	}
};