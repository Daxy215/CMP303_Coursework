#include "ServerHandler.h"

#include <iostream>

ServerHandler::ServerHandler(std::vector<Tank*>& tanks, std::string serverAddress, unsigned short port)
	: tanks(tanks) {
	this->serverAddress = serverAddress;
	this->port = port;
}

void ServerHandler::connect() {
	// bind the listener to a port
	sf::Socket::Status statusTCP = tcpSocket.connect(serverAddress, port);

	if (statusTCP != sf::Socket::Done) {
		// error...
		std::cout << "ERROR: Couldn't connect to server TCP; " << statusTCP << std::endl;
	}

	sf::Socket::Status statusUDP = udpSocket.bind(sf::Socket::AnyPort, serverAddress);
	
	if (statusUDP != sf::Socket::Done) {
		// error...
		std::cout << "ERROR: Couldn't connect to server UDP; " << statusUDP << std::endl;
	}

	//Send UDP port to server.
	sf::Packet data;
	data << "Joined";
	data << serverAddress.toString();
	data << udpSocket.getLocalPort();

	sendDataTCP(tcpSocket, data);

	std::cout << "Port; " << udpSocket.getLocalPort() << std::endl;

	tcpSocket.setBlocking(false);
	udpSocket.setBlocking(false);
	selector.add(tcpSocket);
	selector.add(udpSocket);
}

void ServerHandler::handleConnections() {
	if (selector.wait(sf::milliseconds(1))) {
		if (selector.isReady(tcpSocket)) { //Handle server data
			//TCP Handling
			sf::Packet packetTCP = receiveDataTCP(tcpSocket);

			if (packetTCP != nullptr) {
				handleTCPData(packetTCP);
			}

			//std::cout << "Received data ;} " << std::endl;
		}
		
		if (selector.isReady(udpSocket)) {
			//UDP Handling
			sf::Packet packetUDP = recieveDataUDP(udpSocket);

			if (packetUDP != nullptr) {
				handleUDPData(packetUDP);
			}
		}
	}
}

void ServerHandler::handleTCPData(sf::Packet packet) {
	std::string data;

	packet >> data;

	if (data._Equal("PlayerJoined")) {
		Tank* tank = new Tank("blue", nullptr);

		float x, y;
		int id;

		packet >> x >> y;
		packet >> id;

		tank->setPosition(x, y);
		tank->m_id = id;

		tanks.push_back(tank);

		std::cout << "New player joined! " << tank->m_id << std::endl;
	}

	if (data._Equal("GameStarted")) {
		int countdown = 0;

		packet >> countdown;

		std::cout << "AYOOO STARTED; " << countdown << std::endl;
	}

	//std::cout << "well?" << data << std::endl;
}

void ServerHandler::handleUDPData(sf::Packet packet) {
	std::string data;

	packet >> data;

	//std::cout << "yooo; " << data << std::endl;

	if (data._Equal("PlayerMoved")) {
		float x, y;
		int id;

		packet >> x >> y;
		packet >> id;

		//TODO; make server(onjoin) send client id to use it here as well.
		//std::cout << "Size; " << tanks.size() << std::endl;
		
		std::cout << id << " has moved\n";

		Tank* tank = getTank(id);

		if (tank != nullptr)
			tank->setPosition(x, y);
		//else
		//	std::cout << "Null :(\n";

		//if(tanks.size() > 1)
			//tanks[1]->setPosition(x, y);
	}
}

void ServerHandler::sendDataTCP(sf::TcpSocket& tcpSocket, sf::Packet packet) {
	sf::Socket::Status status = tcpSocket.send(packet);
	
	if (status != sf::Socket::Done) {
		std::cout << "ERROR: Failed to send data to server!" << std::endl;
	}
	
	//std::cout << "sent TCP " << status << std::endl;
}

sf::Packet ServerHandler::receiveDataTCP(sf::TcpSocket& tcpSocket) {
	sf::Packet packet;

	if (selector.isReady(tcpSocket)) {
		if (tcpSocket.receive(packet) != sf::Socket::Done) {
			std::cout << "ERROR: Reciving packet failed\n";

			return packet;
		}

		return packet;
	}
	
	return packet;
}

void ServerHandler::sendDataUDP(sf::UdpSocket& udpSocket, sf::Packet packet) {
	sf::Socket::Status status = udpSocket.send(packet, "localhost", 54000);//TODO;

	if (status != sf::Socket::Done) {
		std::cout << "ERROR: Failed to send data to server!" << std::endl;
	}

	//std::cout << "sent UDP " << status << " / " << port << std::endl;
}

sf::Packet ServerHandler::recieveDataUDP(sf::UdpSocket& udpSocket) {
	sf::Packet packet;

	if (selector.isReady(udpSocket)) {
		sf::IpAddress serverAddress;
		unsigned short port;

		sf::Socket::Status status = udpSocket.receive(packet, serverAddress, port);
		if (status != sf::Socket::Done) {
			std::cout << "ERROR: Reciving UDP packet failed; " << status << std::endl;

			return packet;
		}

		//std::cout << "Recieved from; " << serverAddress << " - " << port << std::endl;

		return packet;
	}

	return packet;
}

Tank* ServerHandler::getTank(int id) {
	for (auto& tank : tanks)
		if (tank->m_id == id)
			return tank;

	return nullptr;
}
