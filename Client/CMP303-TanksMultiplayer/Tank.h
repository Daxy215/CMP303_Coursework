#pragma once
#include <SFML\Graphics.hpp>

#include <vector>
#include "TankMessage.h"

#include <deque>

struct Predication {
public:
	Predication() {
		
	}
	
	Predication(float timeStamp, sf::Vector2f position, sf::Vector2f velocity) {
		this->timeStamp = timeStamp;
		this->position = position;
		this->velocity = velocity;
	}
	
public:
	float timeStamp;
	
	sf::Vector2f position, velocity;
};


class ServerHandler;

class Tank : public sf::Sprite
{
public:
	Tank(std::string color, ServerHandler* serverHandler);
	~Tank();

	enum RenderMode {
		REAL_ONLY,
		PREDICTED_ONLY,
		REAL_AND_PREDICTED
	};

	void UpdateInput(float dt, sf::Keyboard::Key eventType);
	const void Render(sf::RenderWindow* window);

	void SetRenderMode(const RenderMode renderMode) { m_RenderMode = renderMode; }
	//void setPosition(float x, float y);
	void addPosition(float x, float y);
	void addRotation(float angle);

	void setGhostPosition(sf::Vector2f pos);

public:
	int m_id;

	bool m_isReady = false;
	bool m_isDead = false;

	ServerHandler* serverHandler;

	sf::Sprite	m_BarrelSprite;

	std::deque<Predication*> predications;
private:
	float m_speed = 150;

	sf::Sprite	m_GhostSprite;

	sf::Texture m_BodyTexture;
	sf::Texture m_BarrelTexture;
	float		m_BodyRotation;
	float		m_BarrelRotation;

	RenderMode	m_RenderMode = RenderMode::REAL_ONLY;
};

