#pragma once
#include <SFML/Network.hpp>

#include <string>
#include <list>

#include "Tank.h"

class ServerHandler
{
public:
	ServerHandler(std::vector<Tank*>& tanks, std::string serverAddress, unsigned short port);

	void connect();
	void handleConnections();

	void handleTCPData(sf::Packet packet);
	void handleUDPData(sf::Packet packet);

	void sendDataTCP(sf::TcpSocket& tcpSocket, sf::Packet packet);
	sf::Packet receiveDataTCP(sf::TcpSocket& tcpSocket);
	void sendDataUDP(sf::UdpSocket& udpSocket, sf::Packet packet);
	sf::Packet recieveDataUDP(sf::UdpSocket& udpSocket);

public:
	sf::IpAddress serverAddress;
	unsigned short port;

	sf::SocketSelector selector;

	sf::TcpSocket tcpSocket;
	sf::UdpSocket udpSocket;

	std::vector<Tank*>& tanks;
};