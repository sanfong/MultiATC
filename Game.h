#pragma once
#include "ConsoleLog.h"	

namespace Game
{
	class Plane
	{
	public:
		char name;
		Uint16 altitude;
		Uint16 targetAltitude;
		Uint16 destination;
		XY position;
		XY direction;
		bool isActive;
		bool isJet; // + 32
		bool isHolding;
		int gas;
		int holdingCount;
		int boardColor;
		XY beaconPos;
		string instruction;
		inline Plane()
		{
			reset();
		}
		inline void reset()
		{
			name = 'A';
			altitude = 7;
			targetAltitude = 7;
			destination = 0;
			holdingCount = 0;
			gas = 60;
			boardColor = 10;
			isJet = false;
			isActive = false;
			isHolding = false;
		}
		inline void setInfo(Uint16 dest, XY pos, XY dir, bool jet, bool active, bool hold, Uint16 a, Uint16 ta, int g = 60)
		{
			destination = dest;
			position = pos;
			direction = dir;
			isJet = jet;
			isActive = active;
			isHolding = hold;
			altitude = a;
			targetAltitude = ta;
			gas = g;
			holdingCount = 0;
			beaconPos = XY(-1, -1);
			instruction = "";
		}
		inline void receiveInfo(const Plane& plane)
		{
			destination = plane.destination;
			position = plane.position;
			altitude = plane.altitude;
			isJet = plane.isJet;
			isActive = plane.isActive;
			isHolding = plane.isHolding;
		}
		inline void move()
		{
			if (!isActive)
				return;

			if (isHolding)
				holdingCount++;

			if (altitude > targetAltitude)
			{
				altitude--;
				if (altitude == 0)
					position += direction;
			}
			else if (altitude < targetAltitude)
			{
				altitude++;
			}

			if (altitude > 0)
			{
				position += direction;
				isHolding = false;
				gas -= 1;
			}
		}
		inline string setDirection(char d)
		{
			XY newDirection = direction; // bug fixed
			switch (d)
			{
			case 'q': // NW
				newDirection = { -2,-1 };
				break;
			case 'w': // N
				newDirection = { 0,-1 };
				break;
			case 'e': // NE
				newDirection = { 2,-1 };
				break;
			case 'a': // W
				newDirection = { -2,0 };
				break;
			case 'd': // E
				newDirection = { 2,0 };
				break;
			case 'z': // SW
				newDirection = { -2,1 };
				break;
			case 'x': // S
				newDirection = { 0,1 };
				break;
			case 'c': // SE
				newDirection = { 2,1 };
				break;
			case 's':
				return "'s' != 'x'";
				break;
			default:
				return "Invalid direction.";
				break;
			}
			if (newDirection == -direction)
				return "Plane cannot turn 180.";
			else
				direction = newDirection;
			return "";
		}
		inline string getDisplayName() { return char(name + (' ' * isJet)) + to_string(altitude); }
		inline bool isCollidedWith(Plane plane) // Fixed
		{
			XY absXY = abs(position - plane.position);
			return (absXY.x <= 2 && absXY.y <= 1) && (abs(altitude - plane.altitude) <= 1);
		}
		inline string getDestination()
		{
			return (destination < 8) ? "E" + to_string(destination) : "A" + to_string(destination - 8);
		}
		inline void draw()
		{
			console.draw(getDisplayName(), position, 15);
		}
	};
	extern Plane planes[26];
	extern string otherText;
	extern int gameTick;
	extern float countingTick;
	extern float gameTime;
	extern bool gameStart;
	extern int spawnRate;
	extern int countingSpawn;
	extern int score;
	extern int nextCountToSpawn;
	extern int inactivePlane;
	typedef void(*funcptr)(string);
	extern funcptr endCallback;
	void startGame();
	void update(float deltaTime);
	string readCommand(string str);
	void show();
	void endGame(string endMsg = "");
}