#pragma once
#include "Game.h"

#define PORT 53000
extern bool isConnented;

enum class Side
{
	Host,
	Client
} connectSide;

#pragma region ServerData
struct ServerData // Server send data to Client
{
	bool callStartGame;
	bool callEndGame;
	string str;
	bool isCmd;
	bool isGameLog;
	int score;
	Game::Plane planes[26];
};
inline sf::Packet& operator<<(sf::Packet& packet, const Game::Plane& plane)
{
	return packet << plane.altitude << plane.destination << plane.position.x << plane.position.y << plane.isActive << plane.isJet << plane.isHolding;
}
inline sf::Packet& operator>>(sf::Packet& packet, Game::Plane& plane)
{
	return packet >> plane.altitude >> plane.destination >> plane.position.x >> plane.position.y >> plane.isActive >> plane.isJet >> plane.isHolding;
}
inline sf::Packet& operator<<(sf::Packet& packet, const ServerData& data)
{
	packet << data.callStartGame;
	packet << data.callEndGame;
	packet << data.str;
	packet << data.isCmd;
	packet << data.isGameLog;
	packet << data.score;
	if (!data.isCmd || !data.isGameLog || !data.callStartGame || !data.callEndGame)
	{
		for (size_t i = 0; i < 26; i++)
		{
			packet << data.planes[i];
		}
	}
	return packet;
}
inline sf::Packet& operator>>(sf::Packet& packet, ServerData& data)
{
	packet >> data.callStartGame;
	packet >> data.callEndGame;
	packet >> data.str;
	packet >> data.isCmd;
	packet >> data.isGameLog;
	packet >> data.score;
	if (!data.isCmd || !data.isGameLog || !data.callStartGame || !data.callEndGame)
	{
		for (size_t i = 0; i < 26; i++)
		{
			packet >> data.planes[i];
		}
	}
	return packet;
}
#pragma endregion
#pragma region ClientData
struct ClientData // Client send data to Server
{
	bool isCommand;
	string str;
};
sf::Packet& operator <<(sf::Packet& packet, const ClientData& data)
{
	return packet << data.isCommand << data.str;
}
sf::Packet& operator >>(sf::Packet& packet, ClientData& data)
{
	return packet >> data.isCommand >> data.str;
}
#pragma endregion
extern string otherStr;

namespace Server
{
	extern TcpSocket socket;
	inline bool receiveOnce(ClientData& data)
	{
		Packet packet;
		if (socket.receive(packet) != sf::Socket::Done)
		{
			// debug.print("Error: 'Server::receiveOnce' function failed.");
			return false;
		}
		if (!(packet >> data))
		{
			debug.print("Error: failed to read 'data' from the packet", 12);
			return false;
		}
		return true;
	}
	void sendData(const ServerData& data);
	ServerData getInfo(string str, bool isCmd = false);
	inline void receivingData()
	{
		while (console.isOpen() && isConnented)
		{
			ClientData data;
			if (receiveOnce(data)) // get data from client
			{
				// TODO: manage data
				if (data.isCommand)
				{
					debug.print("Other : " + data.str);
					if (data.str.length() > 2)
					{
						string clientLog = Game::readCommand(data.str);
						if (clientLog != "")
						{
							ServerData data = Server::getInfo(clientLog);
							data.isGameLog = true;
							thread sendServerLog(Server::sendData, data);
							sendServerLog.detach();
						}
					}
				}
				else
				{
					otherStr = data.str;
				}
			}
		}
	}
	void callClientEndGame(string str);
	inline void listenForClient()
	{
		debug.print("Waiting for a client to join...", 11);
		TcpListener listener;
		if (listener.listen(PORT) != Socket::Done)
		{
			debug.print("Error: Listening failed.", 12);
			std::this_thread::sleep_for(chrono::seconds(1));
			debug.print("Don't try anything funny.");
			std::this_thread::sleep_for(chrono::seconds(2));
			debug.print("I see what you did there.");
			return;
		}

		if (listener.accept(socket) != sf::Socket::Done)
		{
			debug.print("Error: Cannnot accept client.", 12);
			return;
		}
		listener.close();
		debug.print("Server: Client joined.", 10);
		isConnented = true;
		connectSide = Side::Host;
		thread receiving(Server::receivingData);
		receiving.detach();
		Game::startGame();
		Game::endCallback = callClientEndGame;
	}
	inline void sendData(const ServerData& data)
	{
		static std::mutex mtx;
		if (mtx.try_lock())
		{
			Packet packet;
			packet << data;
			if (socket.send(packet) != sf::Socket::Done)
			{
				debug.print("Error: Send failed.", 12);
				std::this_thread::sleep_for(chrono::seconds(1));
				debug.print("Expected: Client ditched you LOL");
				isConnented = false;
				Game::endCallback = nullptr;
				Game::endGame();
			}
			mtx.unlock();
		}
	}
	inline ServerData getInfo(string str, bool isCmd)
	{
		ServerData data;
		data.callStartGame = false;
		data.callEndGame = false;
		data.str = str;
		data.isCmd = isCmd;
		data.isGameLog = false;
		data.score = Game::score;
		for (size_t i = 0; i < 26; i++)
		{
			data.planes[i] = Game::planes[i];
		}
		return data;
	}
	inline void callClientEndGame(string str)
	{
		ServerData data = Server::getInfo(str);
		data.callEndGame = true;
		data.callStartGame = false;
		thread callFunc(Server::sendData, data);
		callFunc.detach();
	}
	inline void callClientStartGame()
	{
		ServerData data = Server::getInfo("");
		data.callEndGame = false;
		data.callStartGame = true;
		thread callFunc(Server::sendData, data);
		callFunc.detach();
	}
}

namespace Client
{
	extern TcpSocket socket;
	inline bool receiveOnce(ServerData& data)
	{
		Packet packet;
		if (socket.receive(packet) != sf::Socket::Done)
		{
			// debug.print("Error: 'Client::receiveOnce' function failed.");
			return false;
		}
		if (!(packet >> data))
		{
			debug.print("Error: failed to read 'data' from the packet", 12);
			return false;
		}
		return true;
	}
	inline void receivingData()
	{
		while (console.isOpen() && isConnented)
		{
			ServerData data;
			if (receiveOnce(data))
			{
				// TODO: manage data
				if (data.isCmd)
				{
					debug.print("Other : " + data.str);
				}
				else if (data.isGameLog)
				{
					debug.print(data.str, 14);
				}
				else if (data.callStartGame && data.callEndGame)
				{
					debug.print("Error: StartEnd Error.", 12);
				}
				else if (data.callStartGame)
				{
					Game::startGame();
				}
				else if (data.callEndGame)
				{
					Game::endGame(data.str);
				}
				else
				{
					otherStr = data.str;
					Game::score = data.score;
					for (size_t i = 0; i < 26; i++)
					{
						Game::planes[i].receiveInfo(data.planes[i]);
					}
				}
			}
		}
	}
	inline void joinServer(string ip)
	{
		debug.print("Joining to \"" + ip + "\"...", 11);
		Socket::Status status = socket.connect(ip, PORT);
		if (status != sf::Socket::Done)
		{
			debug.print("Error: No server.", 12);
			return;
		}
		debug.print("Client: Joined server.", 10);
		isConnented = true;
		connectSide = Side::Client;
		thread receiving(Client::receivingData);
		receiving.detach();
		Game::startGame();
	}
	inline void sendData(const ClientData& data)
	{
		static std::mutex mtx;
		if (mtx.try_lock())
		{
			Packet packet;
			packet << data;
			if (socket.send(packet) != sf::Socket::Done)
			{
				debug.print("Error: Send failed.", 12);
				std::this_thread::sleep_for(chrono::seconds(1));
				debug.print("Expected: Server dumped you LOL");
				isConnented = false;
				Game::endGame();
			}
			mtx.unlock();
		}
	}
}
// Bug z holding : za3 Kaboom
inline void processCommand(string cmd, bool isCmd)
{
	if (isConnented)
	{
		if (connectSide == Side::Client)
		{
			if (isCmd && cmd.length() == 1)
			{
				Game::readCommand(cmd);
			}
			thread sendStr(Client::sendData, ClientData{ isCmd, cmd });
			sendStr.detach();
		}
		else if (connectSide == Side::Host)
		{
			if (isCmd && cmd == "/restart")
			{
				Server::callClientStartGame();
				Game::startGame();
				return;
			}
			thread sendServerInfo(Server::sendData, Server::getInfo(cmd, isCmd));
			sendServerInfo.detach();
			if (isCmd)
			{
				debug.print(Game::readCommand(cmd), 14);
			}
		}
		else
		{
			debug.print("processCommand: Impossible to get here!", 12);
		}
	}
	else
	{
		if (cmd == "/host")
		{
			thread serverListen(Server::listenForClient);
			serverListen.detach();
			return;
		}
		vector<string> splited = split(cmd);
		if (splited.size() < 2)
			return;
		if (splited[0] == "/join")
		{
			thread clientJoin(Client::joinServer, splited[1]);
			clientJoin.detach();
			return;
		}
	}
}