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

	while (window.isOpen()) {
		//Get the time since the last frame in milliseconds
		float dt = clock.restart().asSeconds() * gameSpeed;

		timer += dt;

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