#pragma once
#include <SFML\Graphics.hpp>
#include <vector>
#include "TankMessage.h"
class Tank : public sf::Sprite
{
public:
	Tank(std::string color);
	~Tank();

	enum RenderMode {
		REAL_ONLY,
		PREDICTED_ONLY,
		REAL_AND_PREDICTED
	};

	const void Render(sf::RenderWindow* window);

	void SetRenderMode(const RenderMode renderMode) { m_RenderMode = renderMode; }
	void setGhostPosition( sf::Vector2f pos );
public:
	sf::Sprite	m_BarrelSprite;
	sf::Sprite	m_GhostSprite;

private:
	sf::Texture m_BodyTexture;
	sf::Texture m_BarrelTexture;
	float		m_BodyRotation;
	float		m_BarrelRotation;

	RenderMode	m_RenderMode = RenderMode::PREDICTED_ONLY;
};

