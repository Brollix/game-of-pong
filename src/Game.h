#pragma once

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
	int maxScore = 5;
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	TTF_Font* font = nullptr;
	TTF_Font* roboto = nullptr;

	GameState state = GameState::MainMenu;

	HUD* hudDisplay = nullptr;
	Text* fitnessInfo = nullptr;
	Text* aiInfoText = nullptr;

	Menu* pauseMenu = nullptr;
	Menu* mainMenu = nullptr;
	Player player;
	Player player2;
	NeuralNet ai;
	Ball* ball = nullptr;

#pragma endregion

#pragma region Constructor

	Game(int w, int h)
		: width(w), height(h),
		player(35, h / 2, 15, 100, { 255, 65, 65, 255 }),
		player2(width - 35, h / 2, 15, 100) {
	}

	~Game() {
		clean();
	}

#pragma endregion

#pragma region Inicialización

	bool init() {
		if (!initSDL()) return false;

		if (ai.loadFromFile("ai_data")) {
			if (ai.bestNet) ai.copyFrom(*ai.bestNet);
		}

		TTF_Font* robotoFont = TTF_OpenFont("assets/roboto.ttf", 18);
		if (!robotoFont) {
			cerr << "Error cargando Roboto: " << TTF_GetError() << endl;
			return false;
		}

		fitnessInfo = new Text(renderer, robotoFont);
		aiInfoText = new Text(renderer, robotoFont, { 20, 60 });

		ball = new Ball(renderer);

		pauseMenu = new Menu( renderer, {
				width / 4, height / 4,
				width / 2, height / 2
			}, {
				"Continue",	"Options", "Main Menu" },
			font
		);

		mainMenu = new Menu( renderer, {
				width / 4, height / 4,
				width / 2, height / 2
			}, {
				"Start Game", "Options", "Quit"	},
			font
		);

		hudDisplay = new HUD(renderer, font);

		return true;
	}

	bool initSDL() {
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
			cerr << "SDL_Init Error: " << SDL_GetError() << endl;
			return false;
		}
		if (TTF_Init() == -1) {
			cerr << "TTF_Init Error: " << TTF_GetError() << endl;
			return false;
		}
		if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
			cerr << "Mix_OpenAudio Error: " << Mix_GetError() << endl;
			return false;
		}

		window = SDL_CreateWindow("Pong SDL2",
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
		if (!window) {
			cerr << "CreateWindow Error: " << SDL_GetError() << endl;
			return false;
		}

		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
		if (!renderer) {
			cerr << "CreateRenderer Error: " << SDL_GetError() << endl;
			return false;
		}

		font = TTF_OpenFont("assets/pong.ttf", 24);
		if (!font) {
			cerr << "TTF_OpenFont Error: " << TTF_GetError() << endl;
			return false;
		}

		return true;
	}


#pragma endregion

#pragma region MainLoop

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

#pragma region Game Loop

	void update(float dt) {
		if (state != GameState::Playing) return;

		player.move(dt);
		ai.update(player2, *ball, dt, player.pos.y);

		stringstream ss;
		ss 
			<< "Fitness: " 
			<< ai.fitness 
			<< "  Best: " 
			<< ai.bestFitness 
			<< "  Gen w/o imp: " 
			<< ai.gensNoImprovement;

		aiInfoText->setString(ss.str());

		ball->move(dt);


		if (Collisions::check(ball->getFRect(), player.getFRect()) ||
			Collisions::check(ball->getFRect(), player2.getFRect())) {
			ball->vel.x *= -1;
		}

		if (Collisions::check(ball->getFRect(), player2.getFRect())) {
			ai.fitness += 5;
			cout << "ai hit the ball" << endl;
		}

		if (ball->pos.x + ball->size.x < 0) {
			player2.score++;
			hudDisplay->setScore(player.score, player2.score);
			resetRound();
		} else if (ball->pos.x > width) {
			player.score++;
			hudDisplay->setScore(player.score, player2.score);
			ai.penalizeForGoal();
			resetRound();
		}

		if (player.score >= maxScore || player2.score >= maxScore) {
			ai.evaluateAndEvolve();
			resetGame();
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
				player2.render(renderer);
				ball->render();
				hudDisplay->render();
				aiInfoText->render();


				break;
			case GameState::Paused:
				player.render(renderer);
				player2.render(renderer);
				ball->render();
				renderPauseOverlay();
				fitnessInfo->render();

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

	void resetRound() {
		ball->pos = { width / 2.0f, height / 2.0f };
		ball->vel = randomVec2iFixed() * ball->speed;
		ai.loadAndApplyBest();
		ai.evaluateAndEvolve();
	}

	void resetGame() {
		// Reset puntuación
		player.score = 0;
		player2.score = 0;
		hudDisplay->setScore(player.score, player2.score);

		// Reset posición de paletas
		player.pos.y = height / 2.0f - player.size.y / 2.0f;
		player2.pos.y = height / 2.0f - player2.size.y / 2.0f;

		// Reset pelota
		resetRound();
	}

#pragma endregion

#pragma region Limpieza

	void clean() {
		if (pauseMenu) delete pauseMenu;
		if (ball) delete ball;
		if (fitnessInfo) delete fitnessInfo;

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
