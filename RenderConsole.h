#pragma once
#include <iostream>
#include <vector>
#include <Windows.h>
#include <time.h>
#include <string.h>
#if __has_include("AsciiText.h")
#include "AsciiText.h"
#endif
#if __has_include("TextBox.h")
#include "TextBox.h"
#endif
using namespace std;

struct ConsoleEvent
{
	WORD type;
	// KeyEvent
	bool keyDown;
	WORD virtualKeyCode;
	char asciiChar;
	// MouseEvent
	bool leftClick;
	bool rightClick;
	bool mouseMoved;
	COORD mousePosition;
	// FocusEvent
	bool setFocus;
	// WindowBufferSizeEvent
	COORD windowBufferSize;
};

class RenderConsole
{
private:
	size_t screenX;
	size_t screenY;
	HANDLE wHnd; // Write Handle
	HANDLE rHnd; // Read Handle
	size_t consoleBufferSize;
	CHAR_INFO* consoleBuffer;
	COORD bufferSize;
	COORD characterPos;
	SMALL_RECT windowSize;

	INPUT_RECORD* eventBuffer;
	DWORD numEvents;
	DWORD numEventsRead;

	bool running;

public:
	inline RenderConsole(size_t width = 120U, size_t height = 30U, const wchar_t* name = L"")
	{
		screenX = width;
		screenY = height;
		running = true;
		wHnd = GetStdHandle(STD_OUTPUT_HANDLE);
		rHnd = GetStdHandle(STD_INPUT_HANDLE);
		consoleBufferSize = width * height;
		consoleBuffer = new CHAR_INFO[consoleBufferSize];
		bufferSize = { (short)screenX,(short)screenY };
		characterPos = { 0,0 };
		windowSize = { 0,0,short((short)screenX - 1),short((short)screenY - 1) };

		numEvents = 0;
		numEventsRead = 0;
		eventBuffer = nullptr;

		if (name[0] != '\0')
		{
			SetConsoleTitleW(name);
		}
		setConsole();
		setMode();
		setcursor(false);
	}
	inline ~RenderConsole()
	{
		if (consoleBuffer != nullptr)
			delete consoleBuffer;
	}
#pragma region Events
	// Call this before getNumEvents()
	inline INPUT_RECORD* getEventBuffer()
	{
		clearEventBuffer();
		GetNumberOfConsoleInputEvents(rHnd, &numEvents);
		if (numEvents == 0)
		{
			eventBuffer = nullptr;
			return eventBuffer;
		}

		eventBuffer = new INPUT_RECORD[numEvents];
		ReadConsoleInput(rHnd, eventBuffer, numEvents, &numEventsRead);
		return eventBuffer;
	}

	// Call after getEventBuffer()
	inline DWORD getNumEventsRead()
	{
		return numEventsRead;
	}

	// ConsoleEvent has KEY_EVENT, MOUSE_EVENT, FOCUS_EVENT, WINDOW_BUFFER_SIZE_EVENT type
	// Should not use with the two above function
	inline bool pollEvents(ConsoleEvent& consoleEv)
	{
		clearEventBuffer();
		GetNumberOfConsoleInputEvents(rHnd, &numEvents);
		if (numEvents == 0)
		{
			eventBuffer = nullptr;
			return false;
		}

		eventBuffer = new INPUT_RECORD[numEvents];
		ReadConsoleInputW(rHnd, eventBuffer, 1, &numEventsRead);
		consoleEv.type = eventBuffer[0].EventType;
		if (consoleEv.type == KEY_EVENT)
		{
			consoleEv.keyDown = eventBuffer[0].Event.KeyEvent.bKeyDown;
			consoleEv.virtualKeyCode = eventBuffer[0].Event.KeyEvent.wVirtualKeyCode;
			consoleEv.asciiChar = eventBuffer[0].Event.KeyEvent.uChar.AsciiChar;

			consoleEv.leftClick = false;
			consoleEv.rightClick = false;
			consoleEv.mouseMoved = false;
			consoleEv.mousePosition = { 0,0 };
		}
		else if (consoleEv.type == MOUSE_EVENT)
		{
			consoleEv.leftClick = eventBuffer[0].Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED;
			consoleEv.rightClick = eventBuffer[0].Event.MouseEvent.dwButtonState & RIGHTMOST_BUTTON_PRESSED;
			consoleEv.mouseMoved = eventBuffer[0].Event.MouseEvent.dwEventFlags & MOUSE_MOVED;
			consoleEv.mousePosition = eventBuffer[0].Event.MouseEvent.dwMousePosition;

			consoleEv.keyDown = false;
			consoleEv.virtualKeyCode = 0;
			consoleEv.asciiChar = 0;
		}
		else if (consoleEv.type == FOCUS_EVENT)
		{
			consoleEv.setFocus = eventBuffer[0].Event.FocusEvent.bSetFocus;
		}
		else if (consoleEv.type == WINDOW_BUFFER_SIZE_EVENT)
		{
			consoleEv.windowBufferSize = eventBuffer[0].Event.WindowBufferSizeEvent.dwSize;
		}
		return true;
	}

	inline bool pollEvents(INPUT_RECORD& evBuffer)
	{
		clearEventBuffer();
		GetNumberOfConsoleInputEvents(rHnd, &numEvents);
		if (numEvents == 0)
		{
			eventBuffer = nullptr;
			return false;
		}

		eventBuffer = new INPUT_RECORD[numEvents];
		ReadConsoleInputW(rHnd, eventBuffer, 1, &numEventsRead);
		evBuffer = eventBuffer[0];
		return true;
	}

	inline void clearEventBuffer()
	{
		if (eventBuffer != nullptr)
		{
			delete eventBuffer;
		}
	}
#pragma endregion

	inline COORD getConsoleSize() { return bufferSize; }
	inline void gotoxy(int x, int y)
	{
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ short(x), short(y) });
	}
	inline void setcursor(bool visible)
	{
		CONSOLE_CURSOR_INFO lpCursor;
		lpCursor.bVisible = visible;
		lpCursor.dwSize = 20;
		SetConsoleCursorInfo(wHnd, &lpCursor);
	}
	inline void setConsole()
	{
		SetConsoleWindowInfo(wHnd, TRUE, &windowSize);
		SetConsoleScreenBufferSize(wHnd, bufferSize);
	}
	inline void setMode(DWORD fdwMode = ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT)
	{
		SetConsoleMode(rHnd, fdwMode);
	}

	inline void clear()
	{
		for (int y = 0; y < screenY; ++y)
		{
			for (int x = 0; x < screenX; ++x)
			{
				consoleBuffer[x + screenX * y].Char.AsciiChar = ' ';
				consoleBuffer[x + screenX * y].Attributes = 7;
			}
		}
	}

#pragma region Draw
	inline void draw(char ch, COORD coord, unsigned short color = 7)
	{
		consoleBuffer[coord.X + screenX * coord.Y].Char.AsciiChar = ch;
		consoleBuffer[coord.X + screenX * coord.Y].Attributes = color;
	}

	inline void draw(const char* strChar, COORD coord, unsigned short color = 7)
	{
		int x = coord.X;
		int y = coord.Y;
		for (size_t i = 0; i < strlen(strChar); i++)
		{
			if (strChar[i] == '\n')
			{
				x = coord.X;
				y++;
				continue;
			}
			if (0 <= x && x < screenX && 0 <= y && y < screenY)
			{
				consoleBuffer[x + screenX * y].Char.AsciiChar = strChar[i];
				consoleBuffer[x + screenX * y].Attributes = color;
			}
			x++;
		}
	}

	inline void draw(string cppstr, COORD coord, unsigned short color = 7)
	{
		int x = coord.X;
		int y = coord.Y;
		for (size_t i = 0; i < cppstr.size(); i++)
		{
			if (cppstr.at(i) == '\n')
			{
				x = coord.X;
				y++;
				continue;
			}
			if (0 <= x && x < screenX && 0 <= y && y < screenY)
			{
				consoleBuffer[x + screenX * y].Char.AsciiChar = cppstr.at(i);
				consoleBuffer[x + screenX * y].Attributes = color;
			}
			x++;
		}
	}

#if __has_include("AsciiText.h")
	inline void draw(AsciiText& texture, COORD coord, unsigned short color = 7)
	{
		int x = coord.X;
		int y = coord.Y;
		for (size_t i = 0; i < texture.texts.size(); i++)
		{
			if (0 <= y && y < screenY)
			{
				for (size_t j = 0; j < texture.texts.at(i).size(); j++)
				{
					if (0 <= x && x < screenX && !(texture.transparent_whitespace && texture.texts.at(i).at(j) == ' '))
					{
						consoleBuffer[x + screenX * y].Char.AsciiChar = texture.texts.at(i).at(j);
						consoleBuffer[x + screenX * y].Attributes = color;
					}
					x++;
				}
			}
			x = coord.X;
			y++;
		}
	}
#endif
#if __has_include("TextBox.h")
	inline void draw(TextBox textbox, unsigned short color = 7)
	{
		if (textbox.isActive)
		{
			draw(textbox.getString(), textbox.getPosition(), color);
		}
	}
#endif
#pragma endregion

	inline void close()
	{
		running = false;
	}

	inline bool isOpen()
	{
		return running;
	}

	inline void display()
	{
		WriteConsoleOutputA(wHnd, consoleBuffer, bufferSize, characterPos, &windowSize);
	}
};