#pragma once

struct Ball {
    SDL_FRect rect;
    SDL_Texture* texture = nullptr;
    SDL_Renderer* renderer = nullptr; // Guardamos el renderer

    Vec2f dir = { 1, 1 };
    float speed = 500;
    int radius = 20;

    Ball(SDL_Renderer* r) : renderer(r) {
        const char* imagePath = "assets/ball.png";

        SDL_Surface* surface = IMG_Load(imagePath);
        if (!surface) {
            SDL_Log("Error cargando imagen de pelota: %s", IMG_GetError());
            return;
        }

        setRenderer(r);

        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        if (!texture) {
            SDL_Log("Error creando textura de pelota: %s", SDL_GetError());
            return;
        }

        rect.x = 640;
        rect.y = 480;
        rect.w = radius;
        rect.h = radius;

        dir = dir.normalized();
    }

    void move(float dt, int winWidth, int winHeight) {
        rect.x += dir.x * speed * dt;
        rect.y += dir.y * speed * dt;

        if (rect.x < 0) {
            rect.x = 0;
            dir.x = -dir.x;
        } else if (rect.x + rect.w > winWidth) {
            rect.x = winWidth - rect.w;
            dir.x = -dir.x;
        }

        if (rect.y < 0) {
            rect.y = 0;
            dir.y = -dir.y;
        } else if (rect.y + rect.h > winHeight) {
            rect.y = winHeight - rect.h;
            dir.y = -dir.y;
        }
    }

    void render() {
        if (renderer && texture) {
            SDL_RenderCopyF(renderer, texture, NULL, &rect);
        }
    }

    SDL_FRect getFRect() const {
        return rect;
    }

    Vec2f getDir() const {
        return dir;
    }

    void setRenderer(SDL_Renderer* r) {
        renderer = r;
    }
};
