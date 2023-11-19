#include "ServerHandler.h"

#include <iostream>

//TODO: Make player send name.

ServerHandler::ServerHandler(std::vector<Tank*>& tanks, std::string serverAddress, unsigned short port) : tanks(tanks) {
	this->serverAddress = serverAddress;
	this->port = port;

	std::mt19937 gens(rd());

	this->gen = gens;
	distribution(gen, 1000);

	float x = 640;
	float y = 480;

	float paddingX = 128;
	float paddingY = 128;

	locations.push_back(new Vector2(paddingX, paddingY));
	locations.push_back(new Vector2(paddingX, y - paddingY));
	locations.push_back(new Vector2(x - paddingX, paddingY));
	locations.push_back(new Vector2(x - paddingX, y - paddingY));
}

void ServerHandler::connect() {
	udpSocket = new sf::UdpSocket();

	// bind the listener to a port
	if (listener.listen(port) != sf::Socket::Done) {
		// error...
		std::cout << "ERROR: Couldn't bind TCP server to port." << std::endl;
	}

	if (udpSocket->bind(port) != sf::Socket::Done) {
		std::cout << "ERROR: Couldn't bind UDP server to port." << std::endl;
	}

	std::cout << "Server TCP/UDP started listening on; " << port << std::endl;

	listener.setBlocking(false);
	//udpSocket->setBlocking(false);

	selector.add(listener);
	selector.add(*udpSocket);
}

void ServerHandler::handleConnections() {
	while(true) {
		if (selector.wait(sf::milliseconds(1))) {
			if (selector.isReady(listener)) {
				sf::TcpSocket* tcpSocket = handleTCP();

				tcpSocket->setBlocking(false);
				selector.add(*tcpSocket);

				Client* client = new Client(distribution(gen), new Player(), tcpSocket);
				clients.push_back(client);

				std::cout << "New client connected! " << client->id << std::endl;

				Tank* tank = new Tank("blue");
				client->player->tank = tank;

				tanks.push_back(client->player->tank);

				sf::Packet welcomePacket;
				welcomePacket << "Welcome";
				welcomePacket << client->id;
			
				sendDataTCP(*client->tcpSocket, welcomePacket);
			
				//No need to notify others.. if there aren't any
				if(clients.size() > 1) {
					//Send to this new client, the previous players
					for (auto& nclient : clients) {
						if (nclient->id == client->id)
							continue;
					
						sf::Packet joinedPacket;
					
						joinedPacket << "PlayerJoined";
						joinedPacket << nclient->player->x << nclient->player->y;
						joinedPacket << nclient->id;
				
						sendDataTCP(*client->tcpSocket, joinedPacket);
					}
				}
			} else {
				for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end();) {
					Client& client = **it;

					//TCP Handling
					Packet packetTCP = receiveDataTCP(*client.tcpSocket);
				
					//Check for connection
					if (packetTCP.status == sf::Socket::Disconnected) {
						std::cout << "Player disconnected" << std::endl;
					
						disconnectClient(&client);
						it = clients.erase(it);
					
						continue;
					}

					//UDP Handling
					Packet packetUDP = recieveDataUDP(client);

					//Handle TCP/UDP data.
					handleTCPData(packetTCP.packet, client);
					handleUDPData(packetUDP.packet, client);

					it++;
				}
			}
		}
	}
}

sf::TcpSocket* ServerHandler::handleTCP() {
	sf::TcpSocket* client = new sf::TcpSocket;

	if (listener.accept(*client) != sf::Socket::Done) {
		delete client;

		return nullptr;
	}

	return client;
}

void ServerHandler::handleGameLogic() {
	if (gameStarted || clients.empty() || clients.size() != MAX_PLAYERS) {
		return;
	}

	//Handle game
	for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end(); it++) {
		Client* client = *it;

		if (!client->player->isReady) {
			return;
		}
	}

	std::cout << "starting.." << std::endl;

	gameStarted = true;

	sf::Packet packet;

	packet << "GameStarted";
	packet << MAXCOUNTDOWNTIME; //Game countdown till start

	startCountdown = true;
	
	sendDataTCPToAllClients(packet);
}

void ServerHandler::handleTCPData(sf::Packet packet, Client& client) {
	std::string data;

	packet >> data;

	if (data._Equal("Joined")) {
		std::string address;
		unsigned short port;

		float x, y;

		packet >> address;
		packet >> port;
		packet >> x >> y;

		client.address = address;
		client.port = port;
		
		client.player->x = x;
		client.player->y = y;
		client.player->tank->setPosition(x, y);
		client.player->tank->m_BarrelSprite.setPosition(x, y);

		//No need to notify others.. if there aren't any
		if (clients.size() > 1) {
			sf::Packet packet;
				
			packet << "PlayerJoined";
			packet << client.player->x << client.player->y;
			packet << client.id;
				
			//Send to all other players that this new client has joined
			sendDataTCPToAllClientsExpect(packet, client.id);
		}
	}
	
	if (data._Equal("Ready")) {
		int id;

		packet >> id;

		//Extra security
		if (client.id != id) {
			return;
		}

		client.player->isReady = !client.player->isReady;

		sf::Packet isReadyPacket;

		isReadyPacket << "IsReady";
		isReadyPacket << client.player->isReady;
		
		sendDataTCP(*client.tcpSocket, isReadyPacket);
		
		handleGameLogic();
	}
}

void ServerHandler::handleUDPData(sf::Packet packet, Client& client) {
	std::string data;

	packet >> data;

	if (data._Equal("Moved")) {
		int id;
		float x, y, r;

		packet >> id >> x >> y >> r;

		if (client.id != id) {
			return;
		}

		client.player->x = x;
		client.player->y = y;

		client.player->tank->setPosition(x, y);
		client.player->tank->m_BarrelSprite.setPosition(x, y);

		client.player->tank->setRotation(r);
		client.player->tank->m_BarrelSprite.setRotation(r);

		//Send to all other clients that this client has moved.
		sf::Packet playerMovedPacket;

		playerMovedPacket << "PlayerMoved";
		playerMovedPacket << x << y << r;
		playerMovedPacket << client.id;

		sendDataUDPToAllClientsExpect(playerMovedPacket, client.id);
	}
}

void ServerHandler::disconnectClient(Client* client) {
	//TODO; Remove tank from vector

	auto it = std::find(tanks.begin(), tanks.end(), client->player->tank);
	if (it != tanks.end()) {
		tanks.erase(it);
	}
	
	selector.remove((*client->tcpSocket));

	client->tcpSocket->disconnect();
}

void ServerHandler::sendDataTCPToAllClients(sf::Packet packet) {
	for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end(); it++) {
		Client& client = **it;

		sf::Socket::Status status = (*client.tcpSocket).send(packet);

		if (status != sf::Socket::Done) {
			std::cout << "ERROR: Failed to send data to client!" << std::endl;
		}
	}
}

void ServerHandler::sendDataTCPToAllClientsExpect(sf::Packet packet, int id) {
	for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end(); it++) {
		Client& client = **it;

		if (client.id == id)
			continue;

		sf::Socket::Status status = client.tcpSocket->send(packet);
		
		if (status != sf::Socket::Done) {
			std::cout << "ERROR: Failed to send data to client!" << std::endl;
		}
	}
}

void ServerHandler::sendDataTCP(sf::TcpSocket& tcpSocket, sf::Packet packet) {
	if (selector.isReady(listener)) {
		sf::Socket::Status status = tcpSocket.send(packet);

		if (status != sf::Socket::Done) {
			std::cout << "ERROR: Failed to send data to client!" << std::endl;
		}
	}
}

Packet ServerHandler::receiveDataTCP(sf::TcpSocket& tcpSocket) {
	sf::Packet packet;

	if (selector.isReady(tcpSocket)) {
		sf::Socket::Status status = tcpSocket.receive(packet);
		if (status != sf::Socket::Done) {
			std::cout << "ERROR: Reciving TCP packet failed; " << status << std::endl;

			return Packet(status);
		}

		return Packet(packet);
	}

	return Packet(packet);
}

void ServerHandler::sendDataUDPToAllClients(sf::Packet packet) {
	for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end(); it++) {
		Client& client = **it;

		sf::Socket::Status status = (*udpSocket).send(packet, client.address, client.port);

		if (status != sf::Socket::Done) {
			std::cout << "ERROR: Failed to send data to client!" << std::endl;
		}
	}
}

void ServerHandler::sendDataUDPToAllClientsExpect(sf::Packet packet, int id) {
	for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end(); it++) {
		Client& client = **it;

		if (client.id == id)
			continue;

		sf::Socket::Status status = (*udpSocket).send(packet, client.address, client.port);

		if (status != sf::Socket::Done) {
			std::cout << "ERROR: Failed to send data to client!" << std::endl;
		}
	}
}

void ServerHandler::sendDataUDP(Client& client, sf::Packet packet) {
	sf::Socket::Status status = (*udpSocket).send(packet, client.address, client.port);

	if (status != sf::Socket::Done) {
		std::cout << "ERROR: Failed to send data to client; " << status << std::endl;
	}

	//std::cout << "Sent; " << status << " - " << client.port << std::endl;
}

Packet ServerHandler::recieveDataUDP(Client& client) {
	sf::Packet packet;

	if (selector.isReady(*udpSocket)) {
		sf::IpAddress clientAddress;
		unsigned short clientPort;

		sf::Socket::Status status = (*udpSocket).receive(packet, clientAddress, clientPort);

		if (status != sf::Socket::Done) {
			std::cout << "ERROR: Reciving UDP packet failed; " << status << std::endl;

			return Packet(status);
		}

		//std::cout << "Got; " << clientAddress << " - " << clientPort << "/" << client.port << std::endl;

		return Packet(packet);
	}

	return Packet(packet);
}
