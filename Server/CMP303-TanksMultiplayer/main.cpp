#include <SFML/Network.hpp>
#include <SFML\Graphics.hpp>
#include <sstream>
#include <iomanip>
#include <thread>

#include <iostream>
#include <string>

#include "ServerHandler.h"
#include "Tank.h"

#define ADDRESS "localhost"
#define PORT 54000

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

void handleCollisions() {
	for(auto& client0 : serverHandler->clients) {
		for(auto& client1 : serverHandler->clients) {
			sf::FloatRect bounds1 = client0->player->tank->getLocalBounds();
			sf::FloatRect bound1G = client0->player->tank->getGlobalBounds();
			sf::FloatRect bounds2 = client1->player->tank->getLocalBounds();

			bounds1.left = client0->player->x;
			bounds1.top = client0->player->y;

			bounds2.left = client1->player->x;
			bounds2.top = client1->player->y;

			if (bounds1.intersects(bounds2)) {
				sf::Packet collisionPacket;

				collisionPacket << "PlayerCollision";
				collisionPacket << client0->id;
				collisionPacket << client1->id; //<- Should be destroyed.

				//Import data, must be sent successfully.
				//serverHandler->sendDataTCPToAllClients(collisionPacket);

				//Remove destroyed client from game
				//serverHandler->disconnectClient(client1);

				std::cout << "died :(\n";
			}
		}
	}
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
	serverHandler = new ServerHandler(tanks, ADDRESS, PORT);
	serverHandler->connect();
	
	//This function will allow for new clients to connect.
	auto handlerFunc = [serverHandler = std::move(serverHandler)]() mutable {
		serverHandler->handleConnections();
	};

	std::thread workerThread(handlerFunc);

	while (window.isOpen()) {
		//Get the time since the last frame in milliseconds
		float dt = clock.restart().asSeconds() * gameSpeed;

		timer += dt;

		if(serverHandler->gameStarted)
			handleCollisions();
		
		if(serverHandler->startCountdown) {
			serverHandler->countdownTimer += dt;
		}
		
		if(serverHandler->countdownTimer >= MAXCOUNTDOWNTIME && serverHandler->startCountdown) {
			serverHandler->startCountdown = false;
			
			//Start game
			std::cout << "STARTING" << std::endl;
			int i = 0;
			for (auto& client : serverHandler->clients) {
				float x = serverHandler->locations[i]->x;
				float y = serverHandler->locations[i]->y;

				sf::Packet startGamePacket;
				startGamePacket << "StartGame";
				startGamePacket << x << y;

				client->player->x = x;
				client->player->y = y;

				client->player->tank->setPosition(x, y);
				client->player->tank->m_BarrelSprite.setPosition(x, y);
				
				sf::Socket::Status status = (*client->tcpSocket).send(startGamePacket);

				sf::Packet playerMovedPacket;

				playerMovedPacket << "PlayerMoved";
				playerMovedPacket << x << y << client->player->tank->getRotation();
				playerMovedPacket << client->id;

				serverHandler->sendDataUDPToAllClientsExpect(playerMovedPacket, client->id);
				
				if (status != sf::Socket::Done) {
					std::cout << "ERROR: Failed to send data to client!" << std::endl;
				}
				
				i++;
			}
			
			serverHandler->gameStarted = true;
		}

		sf::Event event;
		while (window.pollEvent(event))	{
			if (event.type == sf::Event::Closed)
				window.close();
			if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::Key::Escape)
					window.close();
			}
		}
		
		if(!serverHandler->startCountdown) {
			std::string ids = "\n";

			for (auto& client : serverHandler->clients) {
				ids += std::to_string(client->id) + " - ";
				ids += std::to_string(client->player->isReady);
				ids += "\n";
			}
			
			debugText.setString("Game Time: " + Stringify(timer) + " - IDs; " + ids);
		} else {
			debugText.setString("Time left: " + Stringify(MAXCOUNTDOWNTIME - serverHandler->countdownTimer));
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

	workerThread.join();
	
	return 0;
}