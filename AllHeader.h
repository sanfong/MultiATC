#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <thread>
#include <SFML/Audio.hpp>
#include <SFML/Network.hpp>
#include "RenderConsole.h"
#include "Vector2.h"

using namespace sf; // sfml
using namespace std; // standard

extern RenderConsole console;

inline float lenght(VectorXY vec)
{
	return sqrtf(vec.x * vec.x + vec.y * vec.y);
}

inline VectorXY normalize(VectorXY vec)
{
	float len = lenght(vec);
	if (len != 0)
		return vec / len;
	return vec;
}

inline VectorXY abs(VectorXY vec)
{
    return VectorXY(abs(vec.x), abs(vec.y));
}

inline int randint(int start, int stop)
{
    return rand() % (stop - start + 1) + start;
}

inline vector<string> split(const string& s, char delim = ' ')
{
    vector<string> result;
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim))
    {
        result.push_back(item);
    }
    return result;
}