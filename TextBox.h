#pragma once
#include <iostream>
#include <Windows.h>
#include <cctype>
#define set_cursor_at(c) SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c)

using namespace std;

class TextBox
{
private:
	COORD position;
	string beforeStr;
	string afterStr;

	bool selected;

	inline void updateCursor()
	{
		COORD cursorPos = { position.X + beforeStr.length(),position.Y };
		set_cursor_at(cursorPos);
	}

public:
	bool isActive;
	int charLimit;

	inline TextBox(int x = 0, int y = 0)
	{
		position.X = x;
		position.Y = y;
		isActive = true;
		selected = false;
		charLimit = -1;
	}
	inline bool update(char ch, WORD vkCode = 0)
	{
		if (selected && isActive)
		{
			if (ch == '\b')
			{
				if (!beforeStr.empty()) beforeStr.pop_back();
				updateCursor();
			}
			else if (ch == VK_RETURN)
			{
				return true;
			}
			else if (vkCode == VK_LEFT)
			{
				if (!beforeStr.empty())
				{
					afterStr = beforeStr.at(beforeStr.length() - 1) + afterStr;
					beforeStr.pop_back();
				}
				updateCursor();
			}
			else if (vkCode == VK_RIGHT)
			{
				if (!afterStr.empty())
				{
					beforeStr += afterStr.at(0);
					afterStr.erase(afterStr.begin());
				}
				updateCursor();
			}
			else if (vkCode == VK_DELETE)
			{
				if (!afterStr.empty()) afterStr.erase(afterStr.begin());
				updateCursor();
			}
			else if ((charLimit < 0 || (beforeStr + afterStr).length() < charLimit) && isprint(ch))
			{
				beforeStr += ch;
				updateCursor();
			}
		}
		return false;
	}
	inline void setText(string str) 
	{ 
		beforeStr = str;
		afterStr = "";
		updateCursor();
	}
	inline void setPosition(COORD pos) { position = pos; }
	inline void clear() { setText(""); updateCursor(); }
	inline void setSelect(bool select) { selected = select; updateCursor(); }
	inline bool isSelected() { return selected; }
	inline string getString() { return beforeStr + afterStr; }
	inline COORD getPosition() { return position; }
};