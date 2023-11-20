#pragma once
#include <SFML/Network.hpp>

#include <iostream>

#include <string>
#include <vector>
#include <random>

#include "Tank.h"

#define MAX_PLAYERS 4
#define MAXCOUNTDOWNTIME 3

struct Player {
public:
	Player() {
		x = 250;
		y = 250;
	}
	
	void setPosition(float x, float y) {
		this->x = x;
		this->y = y;

		tank->setPosition(x, y);
		tank->m_BarrelSprite.setPosition(x, y);
	}

	void setRotation(float r) {
		tank->setRotation(r);
		tank->m_BarrelSprite.setRotation(r);
	}

public:
	Tank* tank;

	float x,  y;

	bool isReady = false;
	bool isDead = false;
};

struct Client {
public:
	Client(int id, Player* player, sf::TcpSocket* tcpSocket) {
		this->id = id;
		this->player = player;
		this->address = address;
		this->port = port;
		this->tcpSocket = tcpSocket;
	}

public:
	int id;

	Player* player;

	sf::IpAddress address = "";
	unsigned short port;

	sf::TcpSocket* tcpSocket;
};

struct Packet {
public:
	Packet() {}

	Packet(sf::Packet packet) {
		this->packet = packet;
	}

	Packet(sf::Socket::Status status) {
		this->status = status;
	}

public:
	sf::Packet packet;
	sf::Socket::Status status;
};

struct Vector2 {
	Vector2(float x, float y) : x(x), y(y) {}

public:
	float x, y;
};

class ServerHandler
{
public:
	ServerHandler(std::vector<Tank*>& tanks, std::string serverAddress, unsigned short port);

	void connect();
	void handleConnections();
	sf::TcpSocket* handleTCP();

	void handleGameLogic();

	void handleTCPData(sf::Packet packet, Client& client);
	void handleUDPData(sf::Packet packet, Client& client);

	void disconnectClient(Client* client);

	void sendDataTCPToAllClients(sf::Packet packet);
	void sendDataTCPToAllClientsExpect(sf::Packet packet, int id);
	void sendDataTCP(sf::TcpSocket& tcpSocket, sf::Packet packet);
	Packet receiveDataTCP(Client& client);

	void sendDataUDPToAllClients(sf::Packet packet);
	void sendDataUDPToAllClientsExpect(sf::Packet packet, int id);
	void sendDataUDP(Client& client, sf::Packet packet);
	Packet recieveDataUDP(Client& client);

public: //Game variables
	float countdownTimer;
	bool startCountdown = false;
	bool startGame = false;
	bool gameStarted = false;
	
public:
	std::string serverAddress;
	unsigned short port;

	sf::SocketSelector selector;
	sf::UdpSocket* udpSocket;

	sf::TcpListener listener;
	
	std::vector<Client*> clients;
	std::vector<Tank*>& tanks;
	std::vector<Vector2*> locations;

public: //Random variables
	std::random_device rd;
	std::mt19937 gen;
	std::uniform_int_distribution<int> distribution;
};
