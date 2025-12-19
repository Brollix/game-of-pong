#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include "Utils.h"

using namespace std;

struct Ball {
    sf::Sprite sprite;
    sf::Texture texture;
    
    sf::Vector2f dir = { 1, 1 };
    float speed = 500;
    float radius = 20;
    float windowWidth;
    float windowHeight;

    Ball(float windowWidth, float windowHeight) 
        : windowWidth(windowWidth), windowHeight(windowHeight) {
        const char* imagePath = "assets/ball.png";

        if (!texture.loadFromFile(imagePath)) {
            cerr << "Error cargando imagen de pelota: " << imagePath << endl;
        }

        sprite.setTexture(texture);
        sprite.setScale(radius / texture.getSize().x, radius / texture.getSize().y);

        reset();
    }
    
    void reset() {
        // Centrar en pantalla
        sprite.setPosition(windowWidth / 2 - radius / 2, windowHeight / 2 - radius / 2);
        
        // Dirección aleatoria hacia un lado con variación angular
        static bool seeded = false;
        if (!seeded) {
            srand(static_cast<unsigned int>(time(nullptr)));
            seeded = true;
        }
        
        // Generar ángulo aleatorio entre 30 y 60 grados (en radianes)
        const float MIN_ANGLE = 30.0f * 3.14159f / 180.0f;  // 30 grados en radianes
        const float MAX_ANGLE = 60.0f * 3.14159f / 180.0f;  // 60 grados en radianes
        float angleRange = MAX_ANGLE - MIN_ANGLE;
        float randomAngle = MIN_ANGLE + (rand() / (float)RAND_MAX) * angleRange;
        
        // Dirección vertical aleatoria (arriba o abajo)
        float dirY = (rand() % 2 == 0 ? 1.0f : -1.0f);
        
        // Dirección horizontal aleatoria (izquierda o derecha)
        float dirX = (rand() % 2 == 0 ? 1.0f : -1.0f);
        
        // Calcular componentes del vector usando el ángulo
        dir.x = dirX * cos(randomAngle);
        dir.y = dirY * sin(randomAngle);
        
        // Normalizar (por seguridad, aunque ya debería estar normalizado)
        dir = VecUtils::normalized(dir);
    }

    void move(float dt, float winWidth, float winHeight) {
        sf::Vector2f pos = sprite.getPosition();
        pos += dir * speed * dt;

        // Rebote en bordes verticales (arriba/abajo)
        if (pos.y < 0) {
            pos.y = 0;
            dir.y = -dir.y;
        } else if (pos.y + radius > winHeight) {
            pos.y = winHeight - radius;
            dir.y = -dir.y;
        }

        sprite.setPosition(pos);
    }
    
    // Check if ball went out of bounds (scored)
    int checkScore(float winWidth) {
        sf::Vector2f pos = sprite.getPosition();
        if (pos.x < -radius) {
            return 2; // Right player scored
        } else if (pos.x > winWidth) {
            return 1; // Left player scored
        }
        return 0; // No score
    }

    void render(sf::RenderWindow& window) {
        window.draw(sprite);
    }

    sf::FloatRect getBounds() const {
        return sprite.getGlobalBounds();
    }

    sf::Vector2f getDir() const {
        return dir;
    }
};
