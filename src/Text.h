#pragma once

struct Text {

	SDL_Renderer* renderer = nullptr;
	TTF_Font* font = nullptr;
	SDL_Texture* texture = nullptr;
	SDL_Rect rect = { 0, 0, 0, 0 };

	string content;
	SDL_Color color = { 255, 255, 255, 255 };

	Text(SDL_Renderer* r, const char* fontPath, int fontSize) {
		renderer = r;
		font = TTF_OpenFont(fontPath, fontSize);
		if (!font) SDL_Log("Error loading font: %s", TTF_GetError());
	}

	~Text() {
		clean();
	}


	string getText() const {
		return content;
	}

	void setText(const string& txt) {
		content = txt;
		updateTexture();
	}

	SDL_Point getPosition() const {
		return { rect.x, rect.y };
	}

	void setPosition(int x, int y) {
		rect.x = x;
		rect.y = y;
	}

	SDL_Color getColor() const {
		return color;
	}

	void setColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255) {
		color = { r, g, b, a };
		updateTexture();
	}

	SDL_Rect getRect() const {
		return rect;
	}


	void setFont(const char* path, int size) {
		if (font) TTF_CloseFont(font);
		font = TTF_OpenFont(path, size);
		if (!font) SDL_Log("Error loading font: %s", TTF_GetError());
		updateTexture();
	}

	void render() {
		if (texture) SDL_RenderCopy(renderer, texture, NULL, &rect);
	}

	void updateTexture() {
		if (!font || content.empty()) return;

		if (texture) {
			SDL_DestroyTexture(texture);
			texture = nullptr;
		}

		SDL_Surface* surface = TTF_RenderText_Blended(font, content.c_str(), color);
		if (!surface) {
			SDL_Log("Error renderizando texto: %s", TTF_GetError());
			return;
		}

		texture = SDL_CreateTextureFromSurface(renderer, surface);
		rect.w = surface->w;
		rect.h = surface->h;
		SDL_FreeSurface(surface);
	}

	void clean() {
		if (texture) SDL_DestroyTexture(texture);
		if (font) TTF_CloseFont(font);
	}
};
