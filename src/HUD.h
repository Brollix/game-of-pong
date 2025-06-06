#pragma once

struct HUD {
	SDL_Renderer* renderer = nullptr;
	TTF_Font* font = nullptr;

	int scoreA = 0;
	int scoreB = 0;
	SDL_Color color = { 255, 255, 255, 255 };

	HUD(SDL_Renderer* r, TTF_Font* f) {
		renderer = r;
		font = f;
	}

	void setScore(int a, int b) {
		scoreA = a;
		scoreB = b;
	}

	void render() {
		string text = to_string(scoreA) + " : " + to_string(scoreB);
		SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
		SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

		SDL_Rect dst;
		dst.x = 20;
		dst.y = 20;
		dst.w = surface->w;
		dst.h = surface->h;

		SDL_RenderCopy(renderer, texture, NULL, &dst);

		SDL_FreeSurface(surface);
		SDL_DestroyTexture(texture);
	}
};
