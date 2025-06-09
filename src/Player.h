#pragma once

struct Player {
    Vec2f pos;
    Vec2f size;
    SDL_Color color;
    float speed = 300;
    int score = 0;

    Player(float x, float y, float w, float h, SDL_Color rgba = { 255, 255, 255, 255 }) {
        pos = { x, y };
        size = { w, h };
        color = rgba;

        cout << pos << endl;
    }

    void move(float dt) {
        const Uint8* keystates = SDL_GetKeyboardState(NULL);
        if (keystates[SDL_SCANCODE_W]) moveUp(dt);
        if (keystates[SDL_SCANCODE_S]) moveDown(dt);
    }

    void moveUp(float dt) {
        pos.y -= speed * dt;
        clamp(height);
    }

    void moveDown(float dt) {
        pos.y += speed * dt;
        clamp(height);
    }

    void clamp(int windowHeight) {
        if (pos.y < 0) pos.y = 0;
        if (pos.y + size.y > windowHeight) pos.y = windowHeight - size.y;
    }

    void render(SDL_Renderer* renderer) const {
        SDL_FRect rect = getFRect();
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRectF(renderer, &rect);
    }

    SDL_FRect getFRect() const {
        return { pos.x, pos.y, size.x, size.y };
    }
};
