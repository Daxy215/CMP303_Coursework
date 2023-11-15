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


Tank::~Tank()
{
}

//Sets the tank's position to the latest network position
void Tank::Update(float dt)
{
	if (m_Messages.size() < 1)
		return;

	TankMessage latestMessage = m_Messages.back();
	setPosition( latestMessage.x, latestMessage.y );
}

void Tank::UpdateInput(float dt, sf::Keyboard::Key eventType) {
	if (serverHandler == nullptr)
		return;

	if (eventType == sf::Keyboard::Key::W || eventType == sf::Keyboard::Key::Up) {
		addPosition(0, -(m_speed * dt));
	}

	if (eventType == sf::Keyboard::Key::S || eventType == sf::Keyboard::Key::Down) {
		addPosition(0, (m_speed * dt));
	}
	
	if (eventType == sf::Keyboard::Key::A || eventType == sf::Keyboard::Key::Left) {
		addPosition(-(m_speed * dt), 0);
	}

	if (eventType == sf::Keyboard::Key::D || eventType == sf::Keyboard::Key::Right) {
		addPosition((m_speed * dt), 0);
	}

	sf::Packet packet;
	packet << "Moved";
	packet << m_id;
	packet << getPosition().x << getPosition().y;

	serverHandler->sendDataUDP(serverHandler->udpSocket, packet);
}

/*void Tank::setPosition(float x, float y) {
	
}*/

void Tank::addPosition(float x, float y) {
	sf::Vector2f position = getPosition();

	position.x += x;
	position.y += y;

	m_BarrelSprite.setPosition(position.x, position.y);
	setPosition(position.x, position.y);
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

//Add a message to the tank's network message queue
void Tank::AddMessage(const TankMessage & msg) {
	m_Messages.push_back(msg);
}

//This method calculates and stores the position, but also returns it immediately for use in the main loop
//This is my where prediction would be... IF I HAD ANY
sf::Vector2f Tank::RunPrediction(float gameTime) {
	float predictedX = -1.0f;
	float predictedY = -1.0f;


	const int msize = m_Messages.size();
	if( msize < 3 ) {
		return sf::Vector2f( predictedX, predictedX );
	}
	const TankMessage& msg0 = m_Messages[msize - 1];
	const TankMessage& msg1 = m_Messages[msize - 2];
	const TankMessage& msg2 = m_Messages[msize - 3];
	
	// FIXME: Implement prediction here!
	// You have:
	// - the history of position messages received, in "m_Messages"
	//   (msg0 is the most recent, msg1 the 2nd most recent, msg2 the 3rd most recent)
	// - the current time, in "gameTime"
	//
	// You need to update:
	// - the predicted position at the current time, in "predictedX" and "predictedY"
		
	return sf::Vector2f( predictedX, predictedY );
}

void Tank::Reset() {
	m_Messages.clear();
}
