#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

using namespace std;

struct Menu {
	sf::RectangleShape background;
	sf::FloatRect rect;

	vector<string> options;
	int selected = 0;

	sf::Font* font;

	// Para guardar las posiciones de cada item en pantalla
	vector<sf::FloatRect> itemRects;
	vector<sf::Text> texts;

	Menu(sf::FloatRect area, vector<string> items, sf::Font* f) {
		rect = area;
		options = items;
		font = f;
		itemRects.resize(options.size());
		texts.resize(options.size());

		background.setPosition(rect.left, rect.top);
		background.setSize(sf::Vector2f(rect.width, rect.height));
		background.setFillColor(sf::Color(30, 30, 30));

		// Preparar textos
		for (int i = 0; i < options.size(); i++) {
			texts[i].setFont(*font);
			texts[i].setString(options[i]);
			texts[i].setCharacterSize(24);
		}
	}

	void update(const sf::Event& event, const sf::Vector2i& mousePos) {
		// Detectar hover
		for (int i = 0; i < itemRects.size(); i++) {
			if (itemRects[i].contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
				selected = i;
				break;
			}
		}

		if (event.type == sf::Event::KeyPressed) {
			if (event.key.code == sf::Keyboard::Up) {
				selected = (selected - 1 + options.size()) % options.size();
			}
			if (event.key.code == sf::Keyboard::Down) {
				selected = (selected + 1) % options.size();
			}
		}

		if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
			for (int i = 0; i < itemRects.size(); i++) {
				if (itemRects[i].contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
					selected = i;
					break;
				}
			}
		}
	}

	void render(sf::RenderWindow& window) {
		window.draw(background);

		float itemHeight = rect.height / options.size();

		for (int i = 0; i < options.size(); i++) {
			sf::Color color = (i == selected)
				? sf::Color(255, 65, 65)
				: sf::Color(200, 200, 200);

			texts[i].setFillColor(color);

			sf::FloatRect textBounds = texts[i].getLocalBounds();
			texts[i].setOrigin(textBounds.left + textBounds.width / 2, textBounds.top + textBounds.height / 2);
			texts[i].setPosition(
				rect.left + rect.width / 2,
				rect.top + i * itemHeight + itemHeight / 2
			);

			// Guardar posición para detección de mouse
			itemRects[i] = texts[i].getGlobalBounds();

			window.draw(texts[i]);

			// Subrayado si está seleccionado
			if (i == selected) {
				sf::RectangleShape underline;
				underline.setSize(sf::Vector2f(textBounds.width, 2));
				underline.setPosition(texts[i].getPosition().x - textBounds.width / 2, 
									  texts[i].getPosition().y + textBounds.height / 2 + 5);
				underline.setFillColor(sf::Color(255, 65, 65));
				window.draw(underline);
			}
		}
	}

	int getSelected() {
		return selected;
	}

	string getOption() {
		return options[selected];
	}
};
