#include "Game.h"
#define GAME_TICK 3

using namespace Game;

RenderConsole console(90, 32, L"Multi ATC");
ConsoleLog debug(XY(10, 25), 6);

Plane Game::planes[26];
string Game::otherText;
int Game::gameTick;
float Game::countingTick;
float Game::gameTime;
int Game::spawnRate;
int Game::countingSpawn;
int Game::score = 0;
int Game::inactivePlane = 26;
int Game::nextCountToSpawn;
bool Game::gameStart = false;
funcptr Game::endCallback = nullptr;

void Game::startGame()
{
	gameStart = true;
	otherText = "";
	gameTick = 0;
	countingTick = 0;
	gameTime = 0;
	score = 0; // forgot to reset score
	spawnRate = 4;
	countingSpawn = 0;
	inactivePlane = 26; // forgot to add inactive plane back
	nextCountToSpawn = 2;
	for (size_t i = 0; i < 26; i++)
	{
		planes[i].reset();
		planes[i].name = 'A' + i;
	}
}

const XY destPoint[10] = { XY(24, 0), XY(58, 0), XY(58, 7), XY(58, 17), XY(18, 20), XY(0, 13), XY(0, 7), XY(0, 0), XY(40, 15), XY(40, 18) };
const XY destOutDir[10] = { XY(0, 1), XY(-2, 1), XY(-2, 0), XY(-2, 0), XY(2, -1), XY(2, 0), XY(2, 0), XY(2, 1), XY(0, -1), XY(2, 0) };
void spawnPlane(int count = 1)
{
	if (inactivePlane == 0 || !(0 < count && count <= 10))
		return;
	vector<int> availableDest = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	for (size_t d = 0; d < count; d++)
	{
		int selectedIndex = rand() % availableDest.size();
		int point = availableDest[selectedIndex];
		availableDest.erase(availableDest.begin() + selectedIndex);
		int dest = (point + randint(1, 9)) % 10;
		bool isJet = randint(0, 1);
		Uint16 altitude = (point < 8) ? 7 : 0;
		bool holding = !altitude;
		for (int i = 0, index = rand() % inactivePlane + 1; i < 26 && index > 0; i++)
		{
			if (!planes[i].isActive)
			{
				if (--index == 0)
				{
					inactivePlane--;
					planes[i].setInfo(dest, destPoint[point], destOutDir[point], isJet, true, holding, altitude, altitude);
				}
			}
		}
	}
}

void Game::update(float deltaTime)
{
	gameTime += deltaTime;
	countingTick += deltaTime;

	if (countingTick >= GAME_TICK)
	{
		countingTick -= GAME_TICK;
		gameTick++;
		countingSpawn++;
		if (countingSpawn >= spawnRate) // fixed
		{
			countingSpawn -= spawnRate;
			spawnPlane(nextCountToSpawn);
			int randomRate = randint(0, 7) * 2; // 0  1  2  3  4  5  6  7
			spawnRate = 8 + randomRate;         // 24 30 36 42 48 54 60 66
			if (0 <= randomRate && randomRate <= 3)
			{
				nextCountToSpawn = 1;
			}
			else if (4 <= randomRate && randomRate <= 6)
			{
				nextCountToSpawn = 2;
			}
			else
			{
				nextCountToSpawn = 3;
			}
		}
		// Update planes
		for (size_t i = 0; i < 26; i++)
		{
			if (gameTick % 2 == 0 || planes[i].isJet)
			{
				planes[i].move();
				if (planes[i].beaconPos != XY(-1, -1) && planes[i].position == planes[i].beaconPos)
				{
					readCommand(planes[i].instruction);
					planes[i].beaconPos = XY(-1, -1);
					planes[i].instruction = "";
				}
			}
		}
		// Condition checking
		for (size_t i = 0; i < 26; i++)
		{
			if (planes[i].holdingCount >= 60)
			{
				endGame(planes[i].getDisplayName() + " stayed at the airport for too long!");
				break;
			}
			if (!planes[i].isActive || planes[i].isHolding) continue; // fix holding

			if (planes[i].gas <= 0)
			{
				endGame(planes[i].getDisplayName() + " runs out of gas!");
				break;
			}
			// Left the area scenario - check
			if (planes[i].position.x == 0 || planes[i].position.x == 58 || planes[i].position.y == 0 || planes[i].position.y == 20)
			{
				planes[i].isActive = false;
				inactivePlane++;
				bool atExit = planes[i].position == destPoint[planes[i].destination];
				bool atAltitude9 = planes[i].altitude == 9;
				bool atDir = planes[i].direction == -destOutDir[planes[i].destination];
				if (atExit && atAltitude9 && atDir)
				{
					score++;
				}
				else
				{
					endGame(planes[i].getDisplayName() + " illegally left the area!");
					break;
				}
			}
			// Landed scenario - fixed
			if (planes[i].altitude == 0)
			{
				bool atAirport = planes[i].position == destPoint[planes[i].destination];
				bool atDir = planes[i].direction == destOutDir[planes[i].destination];
				if (atAirport && atDir && planes[i].destination > 7)
				{
					score++;
					planes[i].isActive = false;
					inactivePlane++;
				}
				else
				{
					endGame(planes[i].getDisplayName() + " dose not landed correctly!");
					break;
				}
			}
			// Check Collision - fixed 5
			for (size_t j = 0; j < 26; j++)
			{
				if (!planes[j].isActive || i == j || planes[j].isHolding) continue;
				if (planes[i].isCollidedWith(planes[j]))
				{
					endGame(planes[i].getDisplayName() + " is collided with " + planes[j].getDisplayName() + "!");
					i = 26;
					break;
				}
			}
		}
	}
}
XY beaconPosition[] = { XY(24, 7), XY(24, 17) };
string Game::readCommand(string str)
{
	Plane* plane = nullptr;
	if (str.empty()) return "";
	for (auto& ch : str) { ch = toupper(ch); }
	if ('A' <= str[0] && str[0] <= 'Z')
		plane = (planes + str[0] - 'A');
	else
		return "";
	if (str.length() == 1)
	{
		for (size_t i = 0; i < 26; i++)
		{
			planes[i].boardColor = 10;
		}
		plane->boardColor = 15;
	}

	if (str.length() >= 6)
	{
		if (str[3] == 'A' && str[4] == 'B' && (str[5] == '0' || str[5] == '1'))
		{
			plane->beaconPos = beaconPosition[str[5] - '0'];
			plane->instruction = str.substr(0, 3);
			return "";
		}
	}

	if (str.length() < 3) return "";
	if (str[1] == 'A')
	{
		if ('0' <= str[2] && str[2] <= '9')
		{
			plane->targetAltitude = str[2] - '0';
		}
	}
	else if (str[1] == 'T')
	{
		if (plane->isHolding)
		{
			return "Landed plane cannot change heading.";
		}
		return plane->setDirection(tolower(str[2]));
	}

	return "";
}

string convertedIGT();

void Game::show()
{
	console.draw(convertedIGT(), XY(67, 0), 10);
	console.draw(to_string(score), XY(67, 1), 10);
	console.draw("Job : " + ((gameStart) ? string("Working") : string("Fired")), XY(61, 2), 3);

	XY writePos(61, 4);
	vector<LogMsg> boardA[2];
	for (size_t i = 0; i < 26; i++)
	{
		if (!planes[i].isActive || planes[i].isHolding)
			continue;
		planes[i].draw();
		console.draw(planes[i].getDisplayName() + "      " + planes[i].getDestination(), writePos, planes[i].boardColor);
		writePos.y++;
	}
	for (size_t i = 0; i < 26; i++)
	{
		if (!(planes[i].isActive && planes[i].isHolding))
			continue;

		if (planes[i].position == destPoint[8])
			boardA[0].push_back({ planes[i].getDisplayName() + "      " + planes[i].getDestination(), planes[i].boardColor });
		else
			boardA[1].push_back({ planes[i].getDisplayName() + "      " + planes[i].getDestination(), planes[i].boardColor });
	}
	writePos.y++;
	for (size_t i = 0; i < 2; i++)
	{
		if (!boardA[i].empty())
		{
			console.draw("Holding at A" + to_string(i) + ":", writePos, 10);
			writePos.y++;
			for (auto& message : boardA[i])
			{
				console.draw(message.str, writePos, message.color);
				writePos.y++;
			}
			writePos.y++;
		}
	}
}

void Game::endGame(string endMsg)
{
	gameStart = false;
	if (endCallback != nullptr)
		endCallback(endMsg);
	debug.print(endMsg, 12);
}

string convertedIGT()
{
	int gameTimeInt = gameTime;
	string draw_time("        ");
	if (gameTimeInt < 60)
	{
		draw_time[0] = '0' + gameTimeInt / 10;
		draw_time[1] = '0' + gameTimeInt % 10;
	}
	else if (gameTimeInt < 3600)
	{
		draw_time[0] = '0' + gameTimeInt / 600;
		draw_time[1] = '0' + (gameTimeInt / 60) % 10;
		draw_time[2] = ':';
		draw_time[3] = '0' + (gameTimeInt / 10) % 6;
		draw_time[4] = '0' + gameTimeInt % 10;
	}
	else
	{
		draw_time[0] = '0' + gameTimeInt / 36000;
		draw_time[1] = '0' + (gameTimeInt / 3600) % 10;
		draw_time[2] = ':';
		draw_time[3] = '0' + (gameTimeInt / 600) % 6;
		draw_time[4] = '0' + (gameTimeInt / 60) % 10;
		draw_time[5] = ':';
		draw_time[6] = '0' + (gameTimeInt / 10) % 6;
		draw_time[7] = '0' + gameTimeInt % 10;
	}
	return draw_time;
}