#pragma once

struct HUD {
	SDL_Renderer* renderer = nullptr;
	TTF_Font* font = nullptr;

	int scoreA = 0;
	int scoreB = 0;
	

	HUD(SDL_Renderer* r, const char* fontPath, int fontSize) {
		renderer = r;
		font = TTF_OpenFont(fontPath, fontSize);

		if (!font) {
			SDL_Log("Error cargando fuente HUD: %s", TTF_GetError());
		}
	}

	~HUD() {
		if (font) TTF_CloseFont(font);
	}

	void setScore(int a, int b) {
		scoreA = a;
		scoreB = b;
	}

	void render() {
		SDL_Color color = { 127, 127, 255, 255 };
		string text = to_string(scoreA) + " : " + to_string(scoreB);

		SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
		SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

		SDL_Rect dst;
		dst.w = surface->w;
		dst.h = surface->h;
		dst.x = (width / 2) - (dst.w / 2);
		dst.y = 20;

		SDL_RenderCopy(renderer, texture, NULL, &dst);

		SDL_FreeSurface(surface);
		SDL_DestroyTexture(texture);
	}
};
