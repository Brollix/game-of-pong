#pragma once

#include <SFML/Graphics.hpp>

struct Player {

	sf::RectangleShape shape;
	float speed = 300;
	int score = 0;

	// Inicializar
	Player(float x, float y, float width, float height) {
		shape.setPosition(x, y);
		shape.setSize(sf::Vector2f(width, height));
		shape.setFillColor(sf::Color(255, 65, 65));
	}

	void move(float windowHeight, float dt) {
		sf::Vector2f pos = shape.getPosition();
		sf::Vector2f size = shape.getSize();

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
			pos.y -= speed * dt;

			if (pos.y < 0) {
				pos.y = 0;
			}
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
			pos.y += speed * dt;

			if (pos.y + size.y > windowHeight) {
				pos.y = windowHeight - size.y;
			}
		}

		shape.setPosition(pos);
	}

	void render(sf::RenderWindow& window) {
		window.draw(shape);
	}

	sf::FloatRect getBounds() const {
		return shape.getGlobalBounds();
	}
};
