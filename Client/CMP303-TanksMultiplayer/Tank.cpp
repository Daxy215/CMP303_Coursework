#include "Tank.h"

#include <iostream>

#include "ServerHandler.h"

Tank::Tank(std::string color, ServerHandler* serverHandler) : sf::Sprite(), serverHandler(serverHandler)
{
	m_BodyTexture.loadFromFile("Assets/" + color + "Tank.png");
	m_BarrelTexture.loadFromFile("Assets/" + color + "Barrel.png");
	setTexture(m_BodyTexture);

	setOrigin(getTextureRect().width / 2, getTextureRect().height / 2);

	m_GhostSprite.setTexture(m_BodyTexture);
	m_GhostSprite.setColor(sf::Color(255, 255, 255, 128));
	m_GhostSprite.setOrigin(getTextureRect().width / 2, getTextureRect().height / 2);
	setGhostPosition( getPosition() );

	m_BarrelSprite.setTexture(m_BarrelTexture);
	m_BarrelSprite.setOrigin(6, 2);
	m_BarrelSprite.setPosition( getPosition() );
}


Tank::~Tank() {
	
}

void Tank::UpdateInput(float dt, sf::Keyboard::Key eventType) {
	if (serverHandler == nullptr)
		return;

	sf::Vector2f velocity = getPosition();

	if (eventType == sf::Keyboard::Key::W || eventType == sf::Keyboard::Key::Up) {
		addPosition(0, -(m_speed * dt));

		setRotation(180);
		m_BarrelSprite.setRotation(180);
	}

	if (eventType == sf::Keyboard::Key::S || eventType == sf::Keyboard::Key::Down) {
		addPosition(0, (m_speed * dt));

		setRotation(0);
		m_BarrelSprite.setRotation(0);
	}
	
	if (eventType == sf::Keyboard::Key::A || eventType == sf::Keyboard::Key::Left) {
		addPosition(-(m_speed * dt), 0);

		setRotation(90);
		m_BarrelSprite.setRotation(90);
	}

	if (eventType == sf::Keyboard::Key::D || eventType == sf::Keyboard::Key::Right) {
		addPosition((m_speed * dt), 0);

		setRotation(-90);
		m_BarrelSprite.setRotation(-90);
	}

	sf::Vector2f difference = velocity - getPosition();
	
	sf::Packet packet;
	packet << "Moved";
	packet << m_id << getPosition().x << getPosition().y << getRotation() << difference.x << difference.y;

	serverHandler->sendDataUDP(serverHandler->udpSocket, packet);
}

void Tank::addPosition(float x, float y) {
	sf::Vector2f position = getPosition();

	position.x += x;
	position.y += y;

	m_BarrelSprite.setPosition(position.x, position.y);
	setPosition(position.x, position.y);
}

void Tank::addRotation(float angle) {
	setRotation(getRotation() + angle);
	m_BarrelSprite.setRotation(m_BarrelRotation + angle);
}

//Use this to set the prediction position
void Tank::setGhostPosition( sf::Vector2f pos ) {
	m_GhostSprite.setPosition( pos );
}

//Draw the tank / or the ghost / or both
const void Tank::Render(sf::RenderWindow * window) {
	if(m_RenderMode > 0)
		window->draw(m_GhostSprite);
	if (m_RenderMode != 1) {
		window->draw(*this);
		window->draw(m_BarrelSprite);
	}
}