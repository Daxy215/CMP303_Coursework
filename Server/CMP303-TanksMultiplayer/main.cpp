#include <SFML/Network.hpp>
#include <SFML\Graphics.hpp>
#include <sstream>
#include <iomanip>

#include <iostream>
#include <string>

#include "ServerHandler.h"
#include "Tank.h"

#define ADDRESS "localhost"
#define PORT 54000

ServerHandler* serverHandlr;
std::vector<Tank*> tanks;

//Rounds a float to two decimal places and turns it into a string
std::string Stringify( float value ) {
	std::stringstream sStream;
	sStream << std::fixed << std::setprecision( 2 ) << value;
	std::string t;
	sStream >> t;
	return t;
}

int main() {
	sf::RenderWindow window(sf::VideoMode(640, 480), "CMP303 - Server");
	window.setFramerateLimit(60);	//Request 60 frames per second

	//Initialise the background texture and sprite
	sf::Texture floorTexture;
	sf::Sprite floor;
	floorTexture.loadFromFile("Assets/tileSand1.png");
	floorTexture.setRepeated(true);
	floor.setTexture(floorTexture);
	floor.setTextureRect(sf::IntRect(0, 0, 640, 480));

	//Initialise font and text
	sf::Font montserrat;
	sf::Text debugText;
	montserrat.loadFromFile("Assets/Montserrat-Regular.ttf");
	debugText.setFont(montserrat);
	debugText.setOutlineColor(sf::Color::Black);
	debugText.setOutlineThickness(1.f);

	//Clock for timing the 'dt' value
	sf::Clock clock;
	float sendRate	= 0.5f;
	float latency	= 0.3f;
	float gameSpeed = 1.0f;
	float startTime = sendRate * 3.0f;

	float timer = 0.0f;

	//Connect to server
	serverHandlr = new ServerHandler(tanks, ADDRESS, PORT);
	serverHandlr->connect();

	/*sf::TcpListener listener;

	// bind the listener to a port
	if (listener.listen(PORT) != sf::Socket::Done)
	{
		// error...
		std::cout << "ff" << std::endl;
	}

	// accept a new connection
	sf::TcpSocket client;
	if (listener.accept(client) != sf::Socket::Done)
	{
		// error...
		std::cout << "ff2" << std::endl;
	}

	//Create and bind UDP Socket
	sf::UdpSocket udpSocket;

	//Bind socket to port
	if (udpSocket.bind(PORT) != sf::Socket::Done) {
		std::cout << "Error binding UDP socket" << std::endl;
	}

	std::cout << "New Client connected to server." << std::endl;

	char data[] = "Welcome to the server bla bla";

	if (client.send(data, 128) != sf::Socket::Done) {
		std::cout << "Sending error" << std::endl;
	}

	std::cout << "Data has been sent\n";

	char d[30];

	sf::Packet packet;
	if (client.receive(packet) != sf::Socket::Done) {
		std::cout << "Error receiving\n";
	}

	packet >> d;

	std::cout << "Received: " << d << std::endl;

	std::cout << "UPD Connection\n";
	sf::IpAddress clientAddress;
	unsigned short clientPort;

	char ud[128];

	std::size_t received;
	if (udpSocket.receive(ud, sizeof(ud), received, clientAddress, clientPort) != sf::Socket::Done)
	{
		// error...
		std::cout << "Error receiving from UDP client" << std::endl;
	}

	std::cout << "Received " << received << " bytes from UDP client: " << ud << std::endl;

	sf::Packet upacket;

	upacket << "WELCOME THROUGH UDP";

	if (udpSocket.send(upacket, clientAddress, clientPort) != sf::Socket::Done) {
		std::cout << "Error sending to UDP client" << std::endl;
	}

	std::cout << "Send to UDP Client" << std::endl;*/

	while (window.isOpen()) {
		//Get the time since the last frame in milliseconds
		float dt = clock.restart().asSeconds() * gameSpeed;

		timer += dt;

		//serverHandlr->handleGameLogic();

		//This function will allow for new clients to connect.
		serverHandlr->handleConnections();

		sf::Event event;
		while (window.pollEvent(event))	{
			if (event.type == sf::Event::Closed)
				window.close();
			if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::Key::Escape)
					window.close();
			}
		}

		//if (timer >= 8 && timer <= 8.02) {
		//	std::cout << "Sending.." << std::endl;
		//	sf::Packet packet;

		//	packet << "AYOOOOOOOOOOOO WELCOME GUYS";

		//	serverHandlr->sendDataTCPToAllClients(packet);
		//}
		
		std::string ids = "\n";

		for (auto& client : serverHandlr->clients) {
			ids += std::to_string(client->id);
			ids += "\n";
		}

		debugText.setString("Game Time: " + Stringify(timer) + " - IDs; " + ids);

		//Render the scene
		window.clear();
		window.draw(floor);

		for (auto& tank : tanks) {
			tank->Render(&window);
		}

		window.draw(debugText);
		window.display();
	}

	return 0;
}