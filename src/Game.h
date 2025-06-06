#pragma once

#include "Utils.h"
#include "Collisions.h"
#include "Menu.h"
#include "HUD.h"
#include "Player.h"
#include "Ball.h"

enum class GameState {
	MainMenu,
	OptionsMenu,
	Playing,
	Paused,
	Exiting
};

#pragma region Game

struct Game {
#pragma region Miembros

	int width, height;
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	TTF_Font* font = nullptr;

	GameState state = GameState::MainMenu;

	HUD* hudDisplay = nullptr;

	Menu* pauseMenu = nullptr;
	Menu* mainMenu = nullptr;
	Player player;
	Ball* ball = nullptr;

#pragma endregion

#pragma region Constructor

	Game(int w, int h)
		: width(w), height(h),
		player(35, h / 2, 15, 100) {
	}

	~Game() {
		clean();
	}

#pragma endregion

#pragma region Inicialización

	bool init() {
		if (!initSDL()) return false;

		ball = new Ball(renderer);

		pauseMenu = new Menu(
			renderer,
			{
				width / 4, height / 4,
				width / 2, height / 2
			},
			{
				"Continue",
				"Options",
				"Main Menu"
			},
			font
		);

		mainMenu = new Menu(
			renderer,
			{
				width / 4, height / 4,
				width / 2, height / 2
			},
			{
				"Start Game",
				"Options",
				"Quit"
			},
			font
		);

		hudDisplay = new HUD(renderer, font);

		return true;
	}

	bool initSDL() {
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
			std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
			return false;
		}
		if (TTF_Init() == -1) {
			std::cerr << "TTF_Init Error: " << TTF_GetError() << std::endl;
			return false;
		}
		if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
			std::cerr << "Mix_OpenAudio Error: " << Mix_GetError() << std::endl;
			return false;
		}

		window = SDL_CreateWindow("Pong SDL2",
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
		if (!window) {
			std::cerr << "CreateWindow Error: " << SDL_GetError() << std::endl;
			return false;
		}

		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
		if (!renderer) {
			std::cerr << "CreateRenderer Error: " << SDL_GetError() << std::endl;
			return false;
		}

		font = TTF_OpenFont("assets/pong.ttf", 24);
		if (!font) {
			std::cerr << "TTF_OpenFont Error: " << TTF_GetError() << std::endl;
			return false;
		}

		return true;
	}


#pragma endregion

#pragma region Loop

	void run() {
		Uint32 lastTime = SDL_GetTicks();

		while (state != GameState::Exiting) {
			Uint32 currentTime = SDL_GetTicks();
			float dt = (currentTime - lastTime) / 1000.0f;
			lastTime = currentTime;

			handleEvents();
			update(dt);
			render();
		}
	}

#pragma endregion

#pragma region Eventos

	void handleEvents() {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				state = GameState::Exiting;
			}

			switch (state) {
				case GameState::MainMenu:
					if (mainMenu) mainMenu->update(event);

					if ((event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN) ||
						(event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)) {

						string opt = mainMenu->getOption();

						if (opt == "Start Game") state = GameState::Playing;
						else if (opt == "Quit") state = GameState::Exiting;
						
					}
					break;

				case GameState::Playing:
					if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
						state = GameState::Paused;
					break;

				case GameState::Paused:
					if (pauseMenu) pauseMenu->update(event);

					if ((event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN) ||
						(event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)) {

						string opt = pauseMenu->getOption();

						if (opt == "Continue") state = GameState::Playing;
						else if (opt == "Quit") state = GameState::Exiting;
						else if (opt == "Main Menu") state = GameState::MainMenu;
					}
					break;

				case GameState::OptionsMenu:

					// "Options" lo ignoro por ahora, lo implemento después

					break;

				default:
					break;
			}
		}
	}


#pragma endregion

#pragma region Lógica

	void update(float dt) {
		if (state != GameState::Playing) return;

		player.move(height, dt);
		ball->move(dt, width, height);

		if (Collisions::check(ball->getFRect(), player.getFRect())) {
		    ball->dir.x *= -1;
		}

		if (true) {

		}
	}

#pragma endregion

#pragma region Render

	void render() {
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		switch (state) {
			case GameState::MainMenu:
				renderMainMenu();
				break;
			case GameState::Playing:
				player.render(renderer);
				ball->render();
				hudDisplay->render();
				break;
			case GameState::Paused:
				player.render(renderer);
				ball->render();
				renderPauseOverlay();
				break;
			default:
				break;
		}

		SDL_RenderPresent(renderer);
		SDL_GL_SwapWindow(window);

	}

	void renderMainMenu() {
		SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
		SDL_Rect menuRect = { width / 4, height / 4, width / 2, height / 2 };
		SDL_RenderFillRect(renderer, &menuRect);

		if (mainMenu) mainMenu->render();
	}

	void renderPauseOverlay() {
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
		SDL_Rect overlay = { 0, 0, width, height };
		SDL_RenderFillRect(renderer, &overlay);

		if (pauseMenu) pauseMenu->render();
	}

#pragma endregion

#pragma region Limpieza

	void clean() {
		if (pauseMenu) delete pauseMenu;
		if (ball) delete ball;
		if (font) TTF_CloseFont(font);

		if (renderer) SDL_DestroyRenderer(renderer);
		if (window) SDL_DestroyWindow(window);

		Mix_CloseAudio();
		Mix_Quit();
		TTF_Quit();
		SDL_Quit();
	}

#pragma endregion
};

#pragma endregion
