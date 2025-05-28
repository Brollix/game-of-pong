#pragma once
#include "SDL_image.h"

struct Ball {
    SDL_FRect rect;
    SDL_Texture* texture = nullptr;

    Vec2f dir = {1, 1};
    float speed = 500;
    int radius = 20;
    
    Ball(SDL_Renderer* renderer) {
        const char* imagePath = "assets/ball.png";

        SDL_Surface* surface = IMG_Load(imagePath);
        
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);  

        rect.x = 640;
        rect.y = 480;
        rect.w = radius;
        rect.h = radius;

        dir = dir.normalized();
    }

    void move(float dt, int winWidth, int winHeight) {
        // Movimiento: dirección * velocidad * dt
        rect.x += dir.x * speed * dt;
        rect.y += dir.y * speed * dt;

        // Rebote horizontal
        if (rect.x < 0) {
            rect.x = 0;
            dir.x = -dir.x;
        } else if (rect.x + rect.w > winWidth) {
            rect.x = winWidth - rect.w;
            dir.x = -dir.x;
        }

        // Rebote vertical
        if (rect.y < 0) {
            rect.y = 0;
            dir.y = -dir.y;
        } else if (rect.y + rect.h > winHeight) {
            rect.y = winHeight - rect.h;
            dir.y = -dir.y;
        }
    }

    void render(SDL_Renderer* renderer) {
        SDL_RenderCopyF(renderer, texture, NULL, &rect);
    }

    SDL_FRect getFRect() {
        return rect;
    }

    Vec2f getDir() {
        return dir;
    }
};
