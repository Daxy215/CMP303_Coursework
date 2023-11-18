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

	tanks.push_back(new Tank("red", nullptr));

	//Connect to server
	serverHandler = new ServerHandler(tanks, ADDRESS, PORT);
	serverHandler->connect();

	tanks[0]->serverHandler = serverHandler;

	while (window.isOpen()) {
		//Get the time since the last frame in milliseconds
  		float dt = clock.restart().asSeconds() * gameSpeed;
		
		timer += dt;
		
		serverHandler->handleConnections();
		
		if (serverHandler->countdownTimer >= serverHandler->maxCountdownTime) {
			serverHandler->startCountdown = false;
			
			//Wait for server to send packet
		}

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
				if(!serverHandler->startCountdown)
					tanks[0]->UpdateInput(dt, event.key.code);

				if (event.key.code == sf::Keyboard::Key::Escape)
					window.close();
				if( event.key.code == sf::Keyboard::Key::R ) {
					std::cout << "--------READY--------\n";

					sf::Packet packet;

					packet << "Ready";
					packet << tanks[0]->m_id;

					serverHandler->sendDataTCP(serverHandler->tcpSocket, packet);
				}
			}
		}
		
		if (!serverHandler->startCountdown) {
			std::string ids = "\n";

			for (auto& tank : tanks) {
				ids += std::to_string(tank->m_id);
				ids += "\n";
			}

			debugText.setString("Game Time: " + Stringify(timer) + " - IDs; " + ids);
		} else {
			debugText.setString("Time left: " + Stringify(serverHandler->maxCountdownTime - serverHandler->countdownTimer));
		}

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