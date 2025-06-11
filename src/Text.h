#pragma once

struct Text {
    SDL_Renderer* renderer = nullptr;
    TTF_Font* font = nullptr;
    SDL_Color color = { 255, 255, 255, 255 };
    string content;
    SDL_FPoint position = { 0, 0 };

    int textWidth = 0;
    int textHeight = 0;

    Text(SDL_Renderer* r, TTF_Font* f, SDL_Color c = { 255, 255, 255 }) 
        : renderer(r), font(f), color(c) {
    }

    void setString(const string& str) {
        content = str;
    }

    void setPosition(float x, float y) {
        position.x = x;
        position.y = y;
    }

    void setColor(SDL_Color c) {
        color = c;
    }

    void render() {
        if (!renderer || !font || content.empty()) return;

        SDL_Surface* surface = TTF_RenderText_Blended(font, content.c_str(), color);
        if (!surface) return;

        textWidth = surface->w;
        textHeight = surface->h;

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FRect dst = { position.x, position.y, (float)textWidth, (float)textHeight };
        SDL_RenderCopyF(renderer, texture, nullptr, &dst);

        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
    }

    int getWidth() { return textWidth; }
    int getHeight() { return textHeight; }
};
