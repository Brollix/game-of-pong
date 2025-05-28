#include "SDL.h"
#include <iostream>

using namespace std;

#include "Utils.h"
#include "Collisions.h"

#include "Player.h"
#include "Ball.h"

int width = 1280, height = 720;

int main(int argc, char* argv[]) {

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		cerr << "Error al inicializar SDL: " << SDL_GetError() << endl;
		return 1;
	}

	SDL_Window* window = SDL_CreateWindow(
		"Mi Ventana SDL2",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		width,
		height,
		SDL_WINDOW_SHOWN
	);

	if (!window) {
		cerr << "Error al crear la ventana: " << SDL_GetError() << endl;
		SDL_Quit();
		return 1;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) {
		cerr << "Error al crear el renderer: " << SDL_GetError() << endl;
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	Player player(35, height / 2, 15, 100);
	Ball ball(renderer);

	Uint32 lastTime = SDL_GetTicks();
	float dt = 0.0f;

	bool running = true;
	SDL_Event event;
	while (running) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = false;
			}
		}

		Uint32 currentTime = SDL_GetTicks();
		dt = (currentTime - lastTime) / 1000.0f;
		lastTime = currentTime;

		player.move(height, dt);

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		player.render(renderer);
		ball.render(renderer);
		ball.move(dt, width, height);

		if (Collisions::checkBetween(ball.getFRect(), player.getFRect())) {
			CollisionSide side = Collisions::checkSide(ball.getFRect(), player.getFRect());

			if (side == CollisionSide::Left || side == CollisionSide::Right) {
				ball.dir.x = -ball.dir.x;  // Rebote horizontal
			} else if (side == CollisionSide::Top || side == CollisionSide::Bottom) {
				ball.dir.y = -ball.dir.y;  // Rebote vertical
			}
		} else if (Collisions::wallH(ball.rect, width)) {
			//Perdiste
		}
		else {
			Collisions::wallV(ball.rect, ball.dir, width);
		}

		/*
		if (Collisions::checkBetween(ball.rect, player2.rect)) {
			CollisionSide side = Collisions::checkSide(ball.rect, player2.rect);

			if (side == CollisionSide::Left || side == CollisionSide::Right) {
				ball.dir.x = -ball.dir.x;
			} else if (side == CollisionSide::Top || side == CollisionSide::Bottom) {
				ball.dir.y = -ball.dir.y;
			}
		}
		*/


		SDL_RenderPresent(renderer);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
