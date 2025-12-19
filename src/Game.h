#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <cstdio>
#include <fstream>

#include "Utils.h"
#include "Collisions.h"
#include "Menu.h"
#include "HUD.h"
#include "Player.h"
#include "Ball.h"
#include "AIPlayer.h"
#include "ModelSaver.h"
#include "TournamentManager.h"

using namespace std;

enum class GameState {
	MainMenu,
	OptionsMenu,
	Playing,
	Paused,
	GameOver,
	Tournament,
	TournamentMenu,
	TournamentConfigMenu,
	Exiting
};

const int WIN_SCORE = 7;

#pragma region Game

struct Game {
#pragma region Miembros

	int width, height;
	sf::RenderWindow window;
	sf::Font font;

	GameState state = GameState::MainMenu;

	HUD* hudDisplay = nullptr;

	Menu* pauseMenu = nullptr;
	Menu* mainMenu = nullptr;
	Menu* optionsMenu = nullptr;
	Menu* gameOverMenu = nullptr;
	Menu* tournamentMenu = nullptr;
	Menu* tournamentConfigMenu = nullptr;
	Player player;
	AIPlayer* opponent = nullptr;
	Ball* ball = nullptr;
	
	TournamentManager* tournamentManager = nullptr;

	sf::Clock clock;
	sf::Clock fpsClock;
	float fpsUpdateTime = 0.0f;
	float currentFPS = 0.0f;
	int frameCount = 0;
	
	// Game state tracking
	bool lastBallHitPlayer = false;
	bool lastBallHitOpponent = false;
	vector<float> lastOpponentState;
	
	// Options state
	DifficultyLevel currentDifficulty = DifficultyLevel::Medium;
	bool trainingMode = true;
	
	// Tournament configuration state
	int tournamentPopulationSize = 16;
	int tournamentMaxGenerations = 50;
	int tournamentPointsPerMatch = 7;
	float tournamentSpeedMultiplier = 10.0f;
	float tournamentElitePercent = 0.25f;
	float tournamentMutationRate = 0.1f;

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
		if (!initSFML()) return false;

		ball = new Ball(width, height);
		opponent = new AIPlayer(width - 50, height / 2, 15, 100, currentDifficulty);
		
		// Try to load top 1 from tournament first, then fallback to regular model
		if (opponent) {
			loadTop1ForPlayerVsAI();
		}

		pauseMenu = new Menu(
			sf::FloatRect(width / 4.f, height / 4.f, width / 2.f, height / 2.f),
			{
				"Continue",
				"Options",
				"Main Menu"
			},
			&font
		);

		mainMenu = new Menu(
			sf::FloatRect(width / 4.f, height / 4.f, width / 2.f, height / 2.f),
			{
				"Start Game",
				"Tournament",
				"Options",
				"Quit"
			},
			&font
		);

		optionsMenu = new Menu(
			sf::FloatRect(width / 4.f, height / 4.f, width / 2.f, height / 2.f),
			{
				"Difficulty: Medium",
				"Training: ON",
				"Save Model",
				"Load Model",
				"Back"
			},
			&font
		);

		gameOverMenu = new Menu(
			sf::FloatRect(width / 4.f, height / 4.f, width / 2.f, height / 2.f),
			{
				"Play Again",
				"Main Menu"
			},
			&font
		);
		
		tournamentMenu = new Menu(
			sf::FloatRect(width / 4.f, height / 4.f, width / 2.f, height / 2.f),
			{
				"Start Tournament",
				"Configure",
				"View Stats",
				"Watch Top Match",
				"Back"
			},
			&font
		);
		
		tournamentConfigMenu = new Menu(
			sf::FloatRect(width / 4.f, height / 4.f, width / 2.f, height / 2.f),
			{
				"Population: 16",
				"Generations: 50",
				"Points/Match: 7",
				"Speed: 10.0x",
				"Elite: 25%",
				"Mutation: 10%",
				"Apply & Back"
			},
			&font
		);

		hudDisplay = new HUD(&font, width);
		tournamentManager = new TournamentManager(width, height);
		
		// Apply initial tournament configuration
		applyTournamentConfig();

		return true;
	}

	bool initSFML() {
		window.create(sf::VideoMode(width, height), "Pong SFML", sf::Style::Titlebar | sf::Style::Close);
		
		// Deshabilitar VSync y límite de FPS para máximo rendimiento
		window.setVerticalSyncEnabled(false);
		window.setFramerateLimit(0); // 0 = sin límite de FPS

		if (!font.loadFromFile("assets/SpaceMono.ttf")) {
			cerr << "Error cargando fuente" << endl;
			return false;
		}

		return true;
	}

#pragma endregion

#pragma region Loop

	void run() {
		while (window.isOpen() && state != GameState::Exiting) {
			float dt = clock.restart().asSeconds();
			
			// Limitar deltaTime máximo para evitar saltos grandes
			// cuando el juego se pausa/resume o hay lag
			const float MAX_DT = 0.1f; // 10 FPS mínimo
			if (dt > MAX_DT) dt = MAX_DT;

			// Calcular FPS cada segundo
			frameCount++;
			fpsUpdateTime += dt;
			if (fpsUpdateTime >= 1.0f) {
				currentFPS = frameCount / fpsUpdateTime;
				frameCount = 0;
				fpsUpdateTime = 0.0f;
				if (hudDisplay) {
					hudDisplay->setFPS(currentFPS);
				}
			}

			handleEvents();
			update(dt);
			render();
		}
	}

#pragma endregion

#pragma region Eventos

	void handleEvents() {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				state = GameState::Exiting;
				window.close();
			}

			sf::Vector2i mousePos = sf::Mouse::getPosition(window);

			switch (state) {
				case GameState::MainMenu:
					if (mainMenu) mainMenu->update(event, mousePos);

					if ((event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Return) ||
						(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)) {

						string opt = mainMenu->getOption();

						if (opt == "Start Game") {
							// Load top 1 model from tournament if available
							loadTop1ForPlayerVsAI();
							
							// Ensure training is enabled
							if (opponent) {
								opponent->setTrainingEnabled(true);
							}
							
							state = GameState::Playing;
							ball->reset();
							player.score = 0;
							if (opponent) opponent->setScore(0);
						}
						else if (opt == "Tournament") {
							state = GameState::TournamentMenu;
						}
						else if (opt == "Options") {
							state = GameState::OptionsMenu;
						}
						else if (opt == "Quit") {
							state = GameState::Exiting;
							window.close();
						}
						
					}
					break;

				case GameState::Playing:
					if (event.type == sf::Event::KeyPressed) {
						if (event.key.code == sf::Keyboard::Escape) {
							state = GameState::Paused;
						}
						else if (event.key.code == sf::Keyboard::S && sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
							// Ctrl+S to save model (solo si hay mejora usando fitness)
							if (opponent) {
								float previousBestFitness = opponent->getAgent().getBestFitness();
								if (opponent->getAgent().checkAndUpdateBestFitness()) {
									float currentFitness = opponent->getAgent().getCurrentFitness();
									float newBestFitness = opponent->getAgent().getBestFitness();
									
									if (newBestFitness > previousBestFitness) {
										ModelSaver::saveModel(opponent->getAgent().getNetwork(), "ai_model.bin");
										ModelSaver::saveMetrics(
											newBestFitness,
											opponent->getWins(),
											opponent->getTotalGames(),
											opponent->getAgent().getEpisodeCount(),
											opponent->getAgent().getEpsilon()
										);
										cout << "Modelo mejorado guardado (Ctrl+S)! Fitness: " << currentFitness 
										     << " (Mejor: " << newBestFitness << "), Epsilon: " 
										     << opponent->getAgent().getEpsilon() << endl;
									} else {
										cout << "No hay mejora. Fitness actual: " << currentFitness 
										     << ", Mejor: " << newBestFitness << endl;
									}
								} else {
									cout << "No hay mejora. Fitness: " << opponent->getAgent().getCurrentFitness() 
									     << ", Mejor: " << opponent->getAgent().getBestFitness() << endl;
								}
							}
						}
						else if (event.key.code == sf::Keyboard::L && sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
							// Ctrl+L to load model
							if (opponent) {
								ModelSaver::loadModel(opponent->getAgent().getNetwork(), "ai_model.bin");
							}
						}
					}
					break;

				case GameState::Paused:
					if (pauseMenu) pauseMenu->update(event, mousePos);

					if ((event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Return) ||
						(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)) {

						string opt = pauseMenu->getOption();

						if (opt == "Continue") state = GameState::Playing;
						else if (opt == "Options") {
							state = GameState::OptionsMenu;
						}
						else if (opt == "Quit") {
							state = GameState::Exiting;
							window.close();
						}
						else if (opt == "Main Menu") state = GameState::MainMenu;
					}
					break;

				case GameState::OptionsMenu:
					if (optionsMenu) optionsMenu->update(event, mousePos);

					if ((event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Return) ||
						(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)) {

						string opt = optionsMenu->getOption();

						if (opt.find("Difficulty") != string::npos) {
							// Cycle through difficulty levels
							if (currentDifficulty == DifficultyLevel::Easy) {
								currentDifficulty = DifficultyLevel::Medium;
								optionsMenu->options[0] = "Difficulty: Medium";
							} else if (currentDifficulty == DifficultyLevel::Medium) {
								currentDifficulty = DifficultyLevel::Hard;
								optionsMenu->options[0] = "Difficulty: Hard";
							} else {
								currentDifficulty = DifficultyLevel::Easy;
								optionsMenu->options[0] = "Difficulty: Easy";
							}
							if (opponent) {
								opponent->setDifficulty(currentDifficulty);
							}
						}
						else if (opt.find("Training") != string::npos) {
							// Toggle training mode
							trainingMode = !trainingMode;
							if (opponent) {
								opponent->setTrainingEnabled(trainingMode);
							}
							optionsMenu->options[1] = trainingMode ? "Training: ON" : "Training: OFF";
						}
						else if (opt == "Save Model") {
							if (opponent) {
								// Solo guardar si hay mejora usando fitness
								float previousBestFitness = opponent->getAgent().getBestFitness();
								if (opponent->getAgent().checkAndUpdateBestFitness()) {
									float currentFitness = opponent->getAgent().getCurrentFitness();
									float newBestFitness = opponent->getAgent().getBestFitness();
									
									if (newBestFitness > previousBestFitness) {
										ModelSaver::saveModel(opponent->getAgent().getNetwork(), "ai_model.bin");
										ModelSaver::saveMetrics(
											newBestFitness,
											opponent->getWins(),
											opponent->getTotalGames(),
											opponent->getAgent().getEpisodeCount(),
											opponent->getAgent().getEpsilon()
										);
										cout << "Modelo guardado manualmente. Fitness: " << currentFitness 
										     << " (Mejor: " << newBestFitness << "), Epsilon: " 
										     << opponent->getAgent().getEpsilon() << endl;
									} else {
										cout << "No hay mejora. Fitness actual: " << currentFitness 
										     << ", Mejor: " << newBestFitness << endl;
									}
								} else {
									cout << "No hay mejora. Fitness: " << opponent->getAgent().getCurrentFitness() 
									     << ", Mejor: " << opponent->getAgent().getBestFitness() << endl;
								}
							}
						}
						else if (opt == "Load Model") {
							if (opponent) {
								ModelSaver::loadModel(opponent->getAgent().getNetwork(), "ai_model.bin");
							}
						}
						else if (opt == "Back") {
							state = GameState::MainMenu;
						}
					}
					break;

				case GameState::GameOver:
					if (gameOverMenu) gameOverMenu->update(event, mousePos);

					if ((event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Return) ||
						(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)) {

						string opt = gameOverMenu->getOption();

						if (opt == "Play Again") {
							resetGame();
						}
						else if (opt == "Main Menu") {
							resetGame();
							state = GameState::MainMenu;
						}
					}
					break;
					
				case GameState::TournamentMenu:
					if (tournamentMenu) tournamentMenu->update(event, mousePos);

					if ((event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Return) ||
						(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)) {

						string opt = tournamentMenu->getOption();

						if (opt == "Start Tournament") {
							if (tournamentManager) {
								tournamentManager->initialize();
								tournamentManager->start();
								state = GameState::Tournament;
							}
						}
						else if (opt == "Configure") {
							state = GameState::TournamentConfigMenu;
							updateTournamentConfigMenuTexts();
						}
						else if (opt == "View Stats") {
							// TODO: Show stats screen
							cout << "Tournament statistics (coming soon)" << endl;
						}
						else if (opt == "Watch Top Match") {
							if (tournamentManager && tournamentManager->getPopulation()->getSize() >= 2) {
								tournamentManager->watchMatch(0, 1);
							}
						}
						else if (opt == "Back") {
							state = GameState::MainMenu;
						}
					}
					break;
					
				case GameState::TournamentConfigMenu:
					if (tournamentConfigMenu) tournamentConfigMenu->update(event, mousePos);

					if ((event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Return) ||
						(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)) {

						string opt = tournamentConfigMenu->getOption();

						if (opt.find("Population") != string::npos) {
							// Cycle: 8, 16, 32, 64
							if (tournamentPopulationSize <= 8) {
								tournamentPopulationSize = 16;
							} else if (tournamentPopulationSize <= 16) {
								tournamentPopulationSize = 32;
							} else if (tournamentPopulationSize <= 32) {
								tournamentPopulationSize = 64;
							} else {
								tournamentPopulationSize = 8;
							}
							char buffer[64];
							snprintf(buffer, sizeof(buffer), "Population: %d", tournamentPopulationSize);
							tournamentConfigMenu->options[0] = buffer;
							tournamentConfigMenu->texts[0].setString(buffer);
						}
						else if (opt.find("Generations") != string::npos) {
							// Cycle: 10, 25, 50, 100
							if (tournamentMaxGenerations <= 10) {
								tournamentMaxGenerations = 25;
							} else if (tournamentMaxGenerations <= 25) {
								tournamentMaxGenerations = 50;
							} else if (tournamentMaxGenerations <= 50) {
								tournamentMaxGenerations = 100;
							} else {
								tournamentMaxGenerations = 10;
							}
							char buffer[64];
							snprintf(buffer, sizeof(buffer), "Generations: %d", tournamentMaxGenerations);
							tournamentConfigMenu->options[1] = buffer;
							tournamentConfigMenu->texts[1].setString(buffer);
						}
						else if (opt.find("Points/Match") != string::npos) {
							// Cycle: 3, 5, 7, 11
							if (tournamentPointsPerMatch <= 3) {
								tournamentPointsPerMatch = 5;
							} else if (tournamentPointsPerMatch <= 5) {
								tournamentPointsPerMatch = 7;
							} else if (tournamentPointsPerMatch <= 7) {
								tournamentPointsPerMatch = 11;
							} else {
								tournamentPointsPerMatch = 3;
							}
							char buffer[64];
							snprintf(buffer, sizeof(buffer), "Points/Match: %d", tournamentPointsPerMatch);
							tournamentConfigMenu->options[2] = buffer;
							tournamentConfigMenu->texts[2].setString(buffer);
						}
						else if (opt.find("Speed") != string::npos) {
							// Cycle: 1.0, 5.0, 10.0, 20.0
							if (tournamentSpeedMultiplier <= 1.5f) {
								tournamentSpeedMultiplier = 5.0f;
							} else if (tournamentSpeedMultiplier <= 7.5f) {
								tournamentSpeedMultiplier = 10.0f;
							} else if (tournamentSpeedMultiplier <= 15.0f) {
								tournamentSpeedMultiplier = 20.0f;
							} else {
								tournamentSpeedMultiplier = 1.0f;
							}
							char buffer[64];
							snprintf(buffer, sizeof(buffer), "Speed: %.1fx", tournamentSpeedMultiplier);
							tournamentConfigMenu->options[3] = buffer;
							tournamentConfigMenu->texts[3].setString(buffer);
						}
						else if (opt.find("Elite") != string::npos) {
							// Cycle: 10%, 25%, 50%
							if (tournamentElitePercent <= 0.15f) {
								tournamentElitePercent = 0.25f;
							} else if (tournamentElitePercent <= 0.375f) {
								tournamentElitePercent = 0.50f;
							} else {
								tournamentElitePercent = 0.10f;
							}
							char buffer[64];
							snprintf(buffer, sizeof(buffer), "Elite: %d%%", (int)(tournamentElitePercent * 100));
							tournamentConfigMenu->options[4] = buffer;
							tournamentConfigMenu->texts[4].setString(buffer);
						}
						else if (opt.find("Mutation") != string::npos) {
							// Cycle: 5%, 10%, 20%
							if (tournamentMutationRate <= 0.075f) {
								tournamentMutationRate = 0.10f;
							} else if (tournamentMutationRate <= 0.15f) {
								tournamentMutationRate = 0.20f;
							} else {
								tournamentMutationRate = 0.05f;
							}
							char buffer[64];
							snprintf(buffer, sizeof(buffer), "Mutation: %d%%", (int)(tournamentMutationRate * 100));
							tournamentConfigMenu->options[5] = buffer;
							tournamentConfigMenu->texts[5].setString(buffer);
						}
						else if (opt == "Apply & Back") {
							applyTournamentConfig();
							state = GameState::TournamentMenu;
						}
					}
					break;
					
				case GameState::Tournament:
					// Handle tournament controls
					if (event.type == sf::Event::KeyPressed) {
						if (event.key.code == sf::Keyboard::Escape) {
							state = GameState::TournamentMenu;
						}
						else if (event.key.code == sf::Keyboard::Space) {
							// Pause/Resume
							if (tournamentManager->getState() == TournamentState::Running) {
								tournamentManager->pause();
							} else if (tournamentManager->getState() == TournamentState::Paused) {
								tournamentManager->resume();
							}
						}
					}
					break;

				default:
					break;
			}
		}
	}


#pragma endregion

#pragma region Lógica

	void update(float dt) {
		if (state == GameState::Tournament) {
			updateTournament(dt);
			return;
		}
		
		if (state != GameState::Playing) return;

		player.move(height, dt);
		opponent->update(*ball, dt, height, width);
		ball->move(dt, width, height);

		// Track previous collision states
		bool currentBallHitPlayer = Collisions::check(ball->getBounds(), player.getBounds());
		bool currentBallHitOpponent = Collisions::check(ball->getBounds(), opponent->getBounds());

		// Handle collisions
		if (currentBallHitPlayer && !lastBallHitPlayer) {
		    ball->dir.x *= -1;
		}
		
		if (currentBallHitOpponent && !lastBallHitOpponent) {
		    ball->dir.x *= -1;
		    // Record experience for opponent (ball was hit by opponent)
		    vector<float> currentState = opponent->getCurrentState(*ball, height);
		    opponent->recordExperience(true, false, false, false, currentState);
		}

		lastBallHitPlayer = currentBallHitPlayer;
		lastBallHitOpponent = currentBallHitOpponent;

		// Check for scoring
		int scoreResult = ball->checkScore(width);
		if (scoreResult == 1) {
			// Left player (human) scored
			player.score++;
			bool gameWon = (player.score >= WIN_SCORE);
			opponent->recordPointExperience(false, gameWon);
			ball->reset();
			lastBallHitPlayer = false;
			lastBallHitOpponent = false;
			
			// Check if game is over
			if (gameWon) {
				// Registrar resultado (AI perdió)
				if (opponent) {
					opponent->recordGameResult(false);
					opponent->trainAfterEpisode();
					
					// Verificar si hay mejora usando fitness (combina win rate reciente y epsilon)
					float previousBestFitness = opponent->getAgent().getBestFitness();
					if (opponent->getAgent().checkAndUpdateBestFitness()) {
						float currentFitness = opponent->getAgent().getCurrentFitness();
						float newBestFitness = opponent->getAgent().getBestFitness();
						
						// Solo guardar si realmente mejoró
						if (newBestFitness > previousBestFitness) {
							// Guardar mejor modelo
							ModelSaver::saveModel(opponent->getAgent().getNetwork(), "ai_model.bin");
							ModelSaver::saveMetrics(
								newBestFitness,
								opponent->getWins(),
								opponent->getTotalGames(),
								opponent->getAgent().getEpisodeCount(),
								opponent->getAgent().getEpsilon()
							);
							cout << "Modelo mejorado guardado! Fitness: " << currentFitness 
							     << " (Mejor: " << newBestFitness << "), Epsilon: " 
							     << opponent->getAgent().getEpsilon() << endl;
						}
					}
				}
				state = GameState::GameOver;
				return; // Stop updating
			}
		} else if (scoreResult == 2) {
			// Right player (AI) scored
			opponent->incrementScore();
			bool gameWon = (opponent->getScore() >= WIN_SCORE);
			opponent->recordPointExperience(true, gameWon);
			ball->reset();
			lastBallHitPlayer = false;
			lastBallHitOpponent = false;
			
			// Check if game is over
			if (gameWon) {
				// Registrar resultado (AI ganó)
				if (opponent) {
					opponent->recordGameResult(true);
					opponent->trainAfterEpisode();
					
					// Verificar si hay mejora usando fitness (combina win rate reciente y epsilon)
					float previousBestFitness = opponent->getAgent().getBestFitness();
					if (opponent->getAgent().checkAndUpdateBestFitness()) {
						float currentFitness = opponent->getAgent().getCurrentFitness();
						float newBestFitness = opponent->getAgent().getBestFitness();
						
						// Solo guardar si realmente mejoró
						if (newBestFitness > previousBestFitness) {
							// Guardar mejor modelo
							ModelSaver::saveModel(opponent->getAgent().getNetwork(), "ai_model.bin");
							ModelSaver::saveMetrics(
								newBestFitness,
								opponent->getWins(),
								opponent->getTotalGames(),
								opponent->getAgent().getEpisodeCount(),
								opponent->getAgent().getEpsilon()
							);
							cout << "Modelo mejorado guardado! Fitness: " << currentFitness 
							     << " (Mejor: " << newBestFitness << "), Epsilon: " 
							     << opponent->getAgent().getEpsilon() << endl;
						}
					}
				}
				state = GameState::GameOver;
				return; // Stop updating
			}
		}

		// Update HUD scores
		hudDisplay->setScore(player.score, opponent->getScore());
		
		// Update epsilon display
		if (opponent) {
			float currentEpsilon = opponent->getAgent().getEpsilon();
			hudDisplay->setEpsilon(currentEpsilon);
		}
		
		// Update last state for next frame
		lastOpponentState = opponent->getCurrentState(*ball, height);
	}

	void resetGame() {
		// Load top 1 model from tournament if available
		loadTop1ForPlayerVsAI();
		
		// Ensure training is enabled
		if (opponent) {
			opponent->setTrainingEnabled(true);
		}
		
		player.score = 0;
		if (opponent) {
			opponent->setScore(0);
		}
		if (ball) {
			ball->reset();
		}
		lastBallHitPlayer = false;
		lastBallHitOpponent = false;
		lastOpponentState.clear();
		state = GameState::Playing;
	}
	
	// Load top 1 model from tournament for player vs AI
	void loadTop1ForPlayerVsAI() {
		if (!opponent) return;
		
		// Try to load top 1 from tournament first (best model from tournament)
		ifstream testFile("models/tournament_top_1.bin", ios::binary);
		if (testFile.is_open()) {
			testFile.close();
			if (ModelSaver::loadModel(opponent->getAgent().getNetwork(), "models/tournament_top_1.bin")) {
				cout << "Loaded top 1 model from tournament for player vs AI" << endl;
				
				// Try to load fitness from params file if available
				ifstream paramsFile("models/tournament_top_1_params.txt");
				if (paramsFile.is_open()) {
					string line;
					float fitness = 0.0f;
					while (getline(paramsFile, line)) {
						if (line.find("Fitness:") != string::npos) {
							sscanf(line.c_str(), "Fitness: %f", &fitness);
							opponent->getAgent().setBestFitness(fitness);
							break;
						}
					}
					paramsFile.close();
				}
				return;  // Successfully loaded tournament model
			}
		}
		
		// Fallback to regular model if tournament model doesn't exist
		ifstream regularModel("ai_model.bin", ios::binary);
		if (regularModel.is_open()) {
			regularModel.close();
			ModelSaver::loadModel(opponent->getAgent().getNetwork(), "ai_model.bin");
			
			// Cargar métricas guardadas y restaurar bestFitness del agente
			float loadedFitness, loadedEpsilon;
			int loadedWins, loadedTotalGames, loadedEpisodes;
			if (ModelSaver::loadMetrics(loadedFitness, loadedWins, loadedTotalGames, loadedEpisodes, loadedEpsilon)) {
				// Restaurar el mejor fitness guardado
				opponent->getAgent().setBestFitness(loadedFitness);
				cout << "Métricas cargadas: Fitness: " << loadedFitness 
				     << ", Epsilon: " << loadedEpsilon
				     << ", Partidas: " << loadedTotalGames << ", Victorias: " << loadedWins << endl;
			}
		} else {
			cout << "No saved model found. Using default AI." << endl;
		}
	}
	
	void updateTournamentConfigMenuTexts() {
		if (!tournamentConfigMenu) return;
		
		char buffer[64];
		snprintf(buffer, sizeof(buffer), "Population: %d", tournamentPopulationSize);
		tournamentConfigMenu->options[0] = buffer;
		
		snprintf(buffer, sizeof(buffer), "Generations: %d", tournamentMaxGenerations);
		tournamentConfigMenu->options[1] = buffer;
		
		snprintf(buffer, sizeof(buffer), "Points/Match: %d", tournamentPointsPerMatch);
		tournamentConfigMenu->options[2] = buffer;
		
		snprintf(buffer, sizeof(buffer), "Speed: %.1fx", tournamentSpeedMultiplier);
		tournamentConfigMenu->options[3] = buffer;
		
		snprintf(buffer, sizeof(buffer), "Elite: %d%%", (int)(tournamentElitePercent * 100));
		tournamentConfigMenu->options[4] = buffer;
		
		snprintf(buffer, sizeof(buffer), "Mutation: %d%%", (int)(tournamentMutationRate * 100));
		tournamentConfigMenu->options[5] = buffer;
		
		// Update text objects
		for (int i = 0; i < tournamentConfigMenu->options.size(); i++) {
			tournamentConfigMenu->texts[i].setString(tournamentConfigMenu->options[i]);
		}
	}
	
	void applyTournamentConfig() {
		if (!tournamentManager) return;
		
		TournamentConfig config;
		config.populationSize = tournamentPopulationSize;
		config.maxGenerations = tournamentMaxGenerations;
		config.pointsPerMatch = tournamentPointsPerMatch;
		config.speedMultiplier = tournamentSpeedMultiplier;
		config.elitePercent = tournamentElitePercent;
		config.mutationRate = tournamentMutationRate;
		config.mode = TournamentMode::Evolutionary;
		
		tournamentManager->setConfig(config);
		// Progress output removed
	}
	
	void updateTournament(float dt) {
		if (!tournamentManager) return;
		
		if (tournamentManager->getState() == TournamentState::Running) {
			// Run one generation per update cycle
			bool hasMore = tournamentManager->runGeneration();
			
			if (!hasMore) {
				tournamentManager->stop();
				state = GameState::TournamentMenu;
			}
		}
	}

#pragma endregion

#pragma region Render

	void render() {
		window.clear(sf::Color::Black);

		switch (state) {
			case GameState::MainMenu:
				renderMainMenu();
				break;
			case GameState::OptionsMenu:
				renderOptionsMenu();
				break;
			case GameState::Playing:
				player.render(window);
				opponent->render(window);
				ball->render(window);
				hudDisplay->render(window);
				break;
			case GameState::Paused:
				player.render(window);
				opponent->render(window);
				ball->render(window);
				renderPauseOverlay();
				break;
			case GameState::GameOver:
				player.render(window);
				opponent->render(window);
				ball->render(window);
				hudDisplay->render(window);
				renderGameOver();
				break;
			case GameState::TournamentMenu:
				renderTournamentMenu();
				break;
			case GameState::TournamentConfigMenu:
				renderTournamentConfigMenu();
				break;
			case GameState::Tournament:
				renderTournament();
				break;
			default:
				break;
		}

		window.display();
	}

	void renderMainMenu() {
		sf::RectangleShape menuBg;
		menuBg.setPosition(width / 4.f, height / 4.f);
		menuBg.setSize(sf::Vector2f(width / 2.f, height / 2.f));
		menuBg.setFillColor(sf::Color(20, 20, 20));
		window.draw(menuBg);

		if (mainMenu) mainMenu->render(window);
	}

	void renderPauseOverlay() {
		sf::RectangleShape overlay;
		overlay.setSize(sf::Vector2f(width, height));
		overlay.setFillColor(sf::Color(0, 0, 0, 180));
		window.draw(overlay);

		if (pauseMenu) pauseMenu->render(window);
	}

	void renderOptionsMenu() {
		sf::RectangleShape menuBg;
		menuBg.setPosition(width / 4.f, height / 4.f);
		menuBg.setSize(sf::Vector2f(width / 2.f, height / 2.f));
		menuBg.setFillColor(sf::Color(20, 20, 20));
		window.draw(menuBg);

		if (optionsMenu) optionsMenu->render(window);
	}

	void renderGameOver() {
		// Overlay oscuro
		sf::RectangleShape overlay;
		overlay.setSize(sf::Vector2f(width, height));
		overlay.setFillColor(sf::Color(0, 0, 0, 200));
		window.draw(overlay);

		// Mensaje de resultado
		sf::Text resultText;
		resultText.setFont(font);
		resultText.setCharacterSize(48);
		resultText.setFillColor(sf::Color::White);
		
		string resultStr;
		if (player.score >= WIN_SCORE) {
			resultStr = "You Win!";
			resultText.setFillColor(sf::Color(100, 255, 100)); // Verde
		} else if (opponent && opponent->getScore() >= WIN_SCORE) {
			resultStr = "AI Wins!";
			resultText.setFillColor(sf::Color(255, 100, 100)); // Rojo
		}
		
		resultText.setString(resultStr);
		sf::FloatRect textBounds = resultText.getLocalBounds();
		resultText.setOrigin(textBounds.left + textBounds.width / 2, textBounds.top + textBounds.height / 2);
		resultText.setPosition(width / 2.f, height / 3.f);
		window.draw(resultText);

		// Score final
		sf::Text scoreText;
		scoreText.setFont(font);
		scoreText.setCharacterSize(32);
		scoreText.setFillColor(sf::Color::White);
		scoreText.setString("Final Score: " + to_string(player.score) + " - " + to_string(opponent ? opponent->getScore() : 0));
		sf::FloatRect scoreBounds = scoreText.getLocalBounds();
		scoreText.setOrigin(scoreBounds.left + scoreBounds.width / 2, scoreBounds.top + scoreBounds.height / 2);
		scoreText.setPosition(width / 2.f, height / 3.f + 80);
		window.draw(scoreText);

		// Menú de fin de partida
		if (gameOverMenu) gameOverMenu->render(window);
	}
	
	void renderTournamentMenu() {
		sf::RectangleShape menuBg;
		menuBg.setPosition(width / 4.f, height / 4.f);
		menuBg.setSize(sf::Vector2f(width / 2.f, height / 2.f));
		menuBg.setFillColor(sf::Color(20, 20, 20));
		window.draw(menuBg);

		if (tournamentMenu) tournamentMenu->render(window);
	}
	
	void renderTournamentConfigMenu() {
		sf::RectangleShape menuBg;
		menuBg.setPosition(width / 4.f, height / 4.f);
		menuBg.setSize(sf::Vector2f(width / 2.f, height / 2.f));
		menuBg.setFillColor(sf::Color(20, 20, 20));
		window.draw(menuBg);

		if (tournamentConfigMenu) tournamentConfigMenu->render(window);
	}
	
	void renderTournament() {
		if (!tournamentManager) return;
		
		// Dark background
		window.clear(sf::Color(10, 10, 20));
		
		// Render tournament stats
		if (hudDisplay) {
			hudDisplay->renderTournamentStats(
				window,
				tournamentManager->getCurrentGeneration(),
				tournamentManager->getMaxGenerations(),
				tournamentManager->getProgress(),
				tournamentManager->getStatusMessage(),
				tournamentManager->getCurrentAverageFitness(),  // Use current instead of last
				tournamentManager->getLastBestFitness(),
				tournamentManager->getAllTimeBestFitness(),
				tournamentManager->getAllTimeBestIndividual(),
				tournamentManager->getFormattedETA()  // Add ETA
			);
			
			// Render top individuals
			auto topIndividuals = tournamentManager->getTopIndividuals(5);
			string championId = "";
			if (tournamentManager->hasAllTimeRecord()) {
				const AIIndividual* champion = tournamentManager->getAllTimeBestIndividual();
				if (champion) {
					championId = champion->id;
				}
			}
			hudDisplay->renderTopIndividuals(window, topIndividuals, 5, championId);
		}
		
		// Controls hint
		sf::Text controlsText;
		controlsText.setFont(font);
		controlsText.setCharacterSize(16);
		controlsText.setFillColor(sf::Color(150, 150, 150));
		controlsText.setString("SPACE: Pause/Resume  |  ESC: Back to Menu");
		controlsText.setPosition(width / 2 - controlsText.getLocalBounds().width / 2, height - 50);
		window.draw(controlsText);
	}

#pragma endregion

#pragma region Limpieza

	void clean() {
		if (pauseMenu) delete pauseMenu;
		if (mainMenu) delete mainMenu;
		if (optionsMenu) delete optionsMenu;
		if (gameOverMenu) delete gameOverMenu;
		if (tournamentMenu) delete tournamentMenu;
		if (tournamentConfigMenu) delete tournamentConfigMenu;
		if (hudDisplay) delete hudDisplay;
		if (ball) delete ball;
		if (opponent) delete opponent;
		if (tournamentManager) delete tournamentManager;
	}

#pragma endregion
};

#pragma endregion
