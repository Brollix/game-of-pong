#pragma once

using namespace std;

struct Menu {
	SDL_Renderer* renderer;
	SDL_Rect rect;

	vector<string> options;
	int selected = 0;

	TTF_Font* font;

	// Para guardar las posiciones de cada item en pantalla
	vector<SDL_Rect> itemRects;

	Menu(SDL_Renderer* r, SDL_Rect area, vector<string> items, TTF_Font* f) {
		renderer = r;
		rect = area;
		options = items;
		font = f;
		itemRects.resize(options.size());
	}

	void update(const SDL_Event& event) {
		int mouseX, mouseY;
		SDL_GetMouseState(&mouseX, &mouseY);

		// Detectar hover
		for (int i = 0; i < itemRects.size(); i++) {
			SDL_Rect mouseRect = { mouseX, mouseY, 1, 1 };
			if (SDL_HasIntersection(&mouseRect, &itemRects[i])) {
				selected = i;
				break;
			}
		}

		if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.sym == SDLK_UP) {
				selected = (selected - 1 + options.size()) % options.size();
			}
			if (event.key.keysym.sym == SDLK_DOWN) {
				selected = (selected + 1) % options.size();
			}
		}

		if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
			for (int i = 0; i < itemRects.size(); i++) {
				SDL_Rect mouseRect = { mouseX, mouseY, 1, 1 };
				if (SDL_HasIntersection(&mouseRect, &itemRects[i])) {
					selected = i;
					// Acción puede ejecutarse en el exterior con getOption()
					break;
				}
			}
		}
	}

	void render() {
		SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
		SDL_RenderFillRect(renderer, &rect);

		int itemHeight = rect.h / (int) options.size();

		for (int i = 0; i < options.size(); i++) {
			SDL_Color color = (i == selected)
				? SDL_Color{ 255, 65, 65, 255 }
			: SDL_Color{ 200, 200, 200, 255 };

			SDL_Surface* surface = TTF_RenderText_Blended(font, options[i].c_str(), color);
			SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

			int textW, textH;
			SDL_QueryTexture(texture, NULL, NULL, &textW, &textH);

			SDL_Rect dst;
			dst.x = rect.x + (rect.w - textW) / 2;
			dst.y = rect.y + i * itemHeight + (itemHeight - textH) / 2;
			dst.w = textW;
			dst.h = textH;

			// Guardar posición para detección de mouse
			itemRects[i] = dst;

			SDL_RenderCopy(renderer, texture, NULL, &dst);

			// Subrayado si está seleccionado
			if (i == selected) {
				SDL_SetRenderDrawColor(renderer, 255, 65, 65, 255);
				SDL_Rect underline = {
					dst.x, dst.y + dst.h,
					dst.w, 2
				};
				SDL_RenderFillRect(renderer, &underline);
			}

			SDL_FreeSurface(surface);
			SDL_DestroyTexture(texture);
		}
	}

	int getSelected() {
		return selected;
	}

	string getOption() {
		return options[selected];
	}
};
