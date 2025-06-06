#pragma once

struct Player {

	SDL_FRect rect;
	float speed = 300;
	int score = 0;

	// Inicializar
	Player(int x, int y, int width, int height) {
		rect.x = x;
		rect.y = y;
		rect.w = width;
		rect.h = height;
	}

	void move(int windowHeight, float dt) {
		const Uint8* keystates = SDL_GetKeyboardState(NULL);

		if (keystates[SDL_SCANCODE_W]) {
			rect.y += -speed * dt;

			if (rect.y < 0) {
				rect.y = 0;
			}
		}

		if (keystates[SDL_SCANCODE_S]) {
			rect.y += speed * dt;

			if (rect.y + rect.h > windowHeight) {
				rect.y = windowHeight - rect.h;
			}
		}
	}

	void render(SDL_Renderer* renderer) const {
		SDL_SetRenderDrawColor(renderer, 255, 65, 65, 255);
		SDL_RenderFillRectF(renderer, &rect);
	}

	SDL_FRect getFRect() const {
		return rect;
	}
};
