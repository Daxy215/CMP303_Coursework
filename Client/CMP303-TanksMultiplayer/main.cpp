#include <SFML/Network.hpp>
#include <SFML\Graphics.hpp>
#include <sstream>
#include <iomanip>

#include <iostream>

#include "ServerHandler.h"
#include "TankMessage.h"

#define ADDRESS "localhost"
#define PORT 54000

bool isFocused = true;

ServerHandler* serverHandler;
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
	sf::RenderWindow window(sf::VideoMode(640, 480), "CMP303 - Client");
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
	serverHandler = new ServerHandler(tanks, ADDRESS, PORT);
	serverHandler->connect();

	/*sf::TcpSocket tcpSocket;
	sf::UdpSocket udpSocket;

	sf::Socket::Status status = tcpSocket.connect(ADDRESS, PORT);
	if (status != sf::Socket::Done)
	{
		// error...
		std::cout << "Error" << std::endl;
	}

	//Send data to server
	sf::Packet packet;

	packet << "AYOOO I HAVE JOINED";

	// TCP socket:
	if (tcpSocket.send(packet) != sf::Socket::Done)
	{
		// error...
	}

	std::cout << "data sent to server" << std::endl;
	
	//Recieve data from server
	std::string d;
	sf::Packet pacekt;

	// TCP socket:
	if (tcpSocket.receive(packet) != sf::Socket::Done)
	{
		// error...
		std::cout << "Error in recieving.." << std::endl;
	}

	packet >> d;

	std::cout << "Received " << packet.getDataSize() << " bytes" << d << std::endl;

	std::cout << "UDP Connection\n";

	char ud[] = "AYOOO I HAVE JOINED FROM UDP";

	if (udpSocket.send(ud, sizeof(ud), ADDRESS, PORT) != sf::Socket::Done) {
		std::cout << "Error sending data to UDP server" << std::endl;
	}

	std::cout << "Data sent to UDP server" << std::endl;

	char UDPDDATA[128];
	sf::Packet UDPPakcet;

	//Recieve data from server
	sf::IpAddress serverAddress = ADDRESS;
	unsigned short serverPort = PORT;

	if (udpSocket.receive(UDPPakcet, serverAddress, serverPort) != sf::Socket::Done) {
		std::cout << "Error receiving data from UDP server" << std::endl;
	}

	UDPPakcet >> UDPDDATA;

	std::cout << "Received " << UDPPakcet.getDataSize() << " bytes from UDP server: " << UDPDDATA << std::endl;*/
	
	tanks.push_back(new Tank("red", serverHandler));

	while (window.isOpen()) {
		//Get the time since the last frame in milliseconds
  		float dt = clock.restart().asSeconds() * gameSpeed;

		timer += dt;

		serverHandler->handleConnections();

		sf::Event event;
		while (window.pollEvent(event))	{
			if (event.type == sf::Event::GainedFocus)
				isFocused = true;

			if (event.type == sf::Event::LostFocus)
				isFocused = false;

			if (event.type == sf::Event::Closed)
				window.close();
			if (event.type == sf::Event::KeyPressed && isFocused) {
				//Update player input
				tanks[0]->UpdateInput(dt, event.key.code);

				if (event.key.code == sf::Keyboard::Key::Escape)
					window.close();
				if( event.key.code == sf::Keyboard::Key::R ) {
					std::cout << "\n--------READY--------\n";

					sf::Packet packet;

					packet << "Ready";

					serverHandler->sendDataTCP(serverHandler->tcpSocket, packet);
				}
			}
		}
		
		if(tanks.size() > 1)
			debugText.setString( "Game Time: " + Stringify( timer ) + " - ID; " + Stringify(tanks[1]->m_id));

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