#include "Server.h"

TcpSocket Server::socket;
TcpSocket Client::socket;
bool isConnented = false;
string otherStr;

int main()
{
	console.setcursor(true);
	srand(time(0));

	Clock clock;
	float deltaTime = 0;
	
	AsciiText map("Map.txt");
	TextBox command(10, 21);
	command.setSelect(true);
	command.charLimit = 40;

	TextBox other(10, 23);

	while (console.isOpen())
	{
		deltaTime = clock.restart().asSeconds();
		ConsoleEvent ev;
		while (console.pollEvents(ev))
		{
			if (ev.type == KEY_EVENT && ev.keyDown)
			{
				if (ev.virtualKeyCode == VK_ESCAPE)
				{
					console.close();
				}
				if (command.update(ev.asciiChar, ev.virtualKeyCode))
				{
					string cmdstr = command.getString();
					debug.print(cmdstr);
					command.clear();
					if (cmdstr != "")
					{
						processCommand(cmdstr, true);
					}
				}
			}
		}

		if (isConnented && Game::gameStart)
		{
			if (connectSide == Side::Host)
			{
				Game::update(deltaTime);
				processCommand(command.getString(), false);
			}
			else if (connectSide == Side::Client)
			{
				Game::gameTime += deltaTime;
				processCommand(command.getString(), false);
			}
		}

		console.clear();
		console.draw(map, XY(0, 0), 10);
		console.draw("command :\n\nother   : " + otherStr, XY(0, 21));
		console.draw(command);
		console.draw(other);
		console.draw("Console :", XY(0, 25));
		debug.showOn(console);
		if (isConnented)
		{
			Game::show();
		}
		console.display();
		Sleep(50);
	}

    return 0;
}