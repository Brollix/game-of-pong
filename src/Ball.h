#pragma once

#include "Utils.h"
#include <SDL.h>
#include <SDL_image.h>

struct Ball {
    Vec2f pos;
    Vec2f vel;
    float radius = 10;
    Vec2f size = { radius, radius };
    float speed = 500.0f;

    SDL_Texture* texture = nullptr;
    SDL_Renderer* renderer = nullptr;

    Ball(SDL_Renderer* r) : renderer(r) {
        const char* imagePath = "assets/ball.png";
        SDL_Surface* surface = IMG_Load(imagePath);
        if (!surface) {
            SDL_Log("Error cargando imagen de pelota: %s", IMG_GetError());
            return;
        }

        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        if (!texture) {
            SDL_Log("Error creando textura de pelota: %s", SDL_GetError());
            return;
        }

        pos = { width / 2.0f, height / 2.0f };
        //vel = randomVec2iFixed() * speed;
        vel = { 1, 1 };

        vel *= speed;
    }

    void move(float dt) {
        pos += vel * dt;

        // Solo rebotar en Y
        if (pos.y < 0) {
            pos.y = 0;
            vel.y = -vel.y;
        } else if (pos.y + size.y > height) {
            pos.y = height - size.y;
            vel.y = -vel.y;
        }

        // No rebotar en X (deja que la lógica de Game lo maneje)
    }


    void render() {
        if (!renderer || !texture) return;
        SDL_FRect dst = getFRect();
        SDL_RenderCopyF(renderer, texture, nullptr, &dst);
    }

    Vec2f getDir() const {
        return vel.normalized();
    }

    Vec2f getVel() const {
        return vel;
    }

    SDL_FRect getFRect() const {
        return { pos.x, pos.y, size.x, size.y };
    }

    void setRenderer(SDL_Renderer* r) {
        renderer = r;
    }
};
