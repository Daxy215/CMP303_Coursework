#pragma once
#include <SFML\Graphics.hpp>

#include <vector>
#include "TankMessage.h"

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

	void Update(float dt);
	void UpdateInput(float dt, sf::Keyboard::Key eventType);
	const void Render(sf::RenderWindow* window);

	void AddMessage(const TankMessage& msg);
	sf::Vector2f RunPrediction(float gameTime);

	void SetRenderMode(const RenderMode renderMode) { m_RenderMode = renderMode; }
	void setPosition(float x, float y);
	void addPosition(float x, float y);

	void setGhostPosition(sf::Vector2f pos);
	void Reset();

private:
	float m_speed = 50;

	sf::Sprite	m_BarrelSprite;
	sf::Sprite	m_GhostSprite;

	sf::Texture m_BodyTexture;
	sf::Texture m_BarrelTexture;
	float		m_BodyRotation;
	float		m_BarrelRotation;

	RenderMode	m_RenderMode = RenderMode::REAL_AND_PREDICTED;

	ServerHandler* serverHandler;

	std::vector<TankMessage> m_Messages;
};

