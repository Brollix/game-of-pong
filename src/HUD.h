#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <cstdio>
#include <vector>
#include "AIPopulation.h"

using namespace std;

struct HUD {
	sf::Font* font = nullptr;
	sf::Text text;
	sf::Text epsilonText;
	sf::Text fpsText;

	int scoreA = 0;
	int scoreB = 0;
	float epsilon = 0.0f;
	float fps = 0.0f;
	int windowWidth = 0;
	int windowHeight = 0;

	HUD(sf::Font* f, int width) {
		font = f;
		windowWidth = width;
		windowHeight = 0;
		text.setFont(*font);
		text.setCharacterSize(24);
		text.setFillColor(sf::Color::White);
		text.setPosition(20, 20);
		
		epsilonText.setFont(*font);
		epsilonText.setCharacterSize(18);
		epsilonText.setFillColor(sf::Color(255, 255, 100)); // Amarillo para destacar
		epsilonText.setPosition(20, 55); // Debajo del score
		
		fpsText.setFont(*font);
		fpsText.setCharacterSize(18);
		fpsText.setFillColor(sf::Color(100, 255, 100)); // Verde para FPS
		fpsText.setPosition(20, 80); // Debajo del epsilon
	}

	void setScore(int a, int b) {
		scoreA = a;
		scoreB = b;
	}

	void setEpsilon(float eps) {
		epsilon = eps;
	}

	void setFPS(float f) {
		fps = f;
	}

	void render(sf::RenderWindow& window) {
		string scoreText = to_string(scoreA) + " : " + to_string(scoreB);
		text.setString(scoreText);
		
		// Center text horizontally
		float textWidth = text.getLocalBounds().width;
		float x = (windowWidth - textWidth) / 2.0f;
		text.setPosition(x, 20);
		
		window.draw(text);
		
		// Mostrar epsilon con 3 decimales
		char epsilonStr[32];
		snprintf(epsilonStr, sizeof(epsilonStr), "Epsilon: %.3f", epsilon);
		epsilonText.setString(epsilonStr);
		window.draw(epsilonText);
		
		// Mostrar FPS
		char fpsStr[32];
		snprintf(fpsStr, sizeof(fpsStr), "FPS: %.0f", fps);
		fpsText.setString(fpsStr);
		window.draw(fpsText);
	}
	
	// Render tournament statistics
	void renderTournamentStats(sf::RenderWindow& window, 
	                           int currentGen, int maxGen, 
	                           float progress, 
	                           const string& status,
	                           float avgFitness, 
	                           float bestFitness,
	                           float allTimeBestFitness,
	                           const AIIndividual* champion,
	                           const string& eta) {
		float startY = 30;
		float lineHeight = 25;
		
		// Title
		sf::Text title;
		title.setFont(*font);
		title.setCharacterSize(32);
		title.setFillColor(sf::Color(255, 200, 100));
		title.setString("Tournament Progress");
		title.setPosition(windowWidth / 2 - title.getLocalBounds().width / 2, startY);
		window.draw(title);
		
		startY += 40;
		
		// Generation info
		sf::Text genText;
		genText.setFont(*font);
		genText.setCharacterSize(24);
		genText.setFillColor(sf::Color::White);
		char genStr[64];
		snprintf(genStr, sizeof(genStr), "Generation: %d / %d", currentGen, maxGen);
		genText.setString(genStr);
		genText.setPosition(windowWidth / 2 - genText.getLocalBounds().width / 2, startY);
		window.draw(genText);
		
		startY += lineHeight;
		
		// Progress bar
		float barWidth = 400;
		float barHeight = 30;
		float barX = windowWidth / 2 - barWidth / 2;
		float barY = startY + 10;
		
		// Background
		sf::RectangleShape progressBg(sf::Vector2f(barWidth, barHeight));
		progressBg.setPosition(barX, barY);
		progressBg.setFillColor(sf::Color(50, 50, 50));
		progressBg.setOutlineColor(sf::Color::White);
		progressBg.setOutlineThickness(2);
		window.draw(progressBg);
		
		// Progress fill
		sf::RectangleShape progressFill(sf::Vector2f(barWidth * progress, barHeight));
		progressFill.setPosition(barX, barY);
		progressFill.setFillColor(sf::Color(100, 200, 100));
		window.draw(progressFill);
		
		// Progress percentage
		sf::Text progressText;
		progressText.setFont(*font);
		progressText.setCharacterSize(18);
		progressText.setFillColor(sf::Color::White);
		char progressStr[32];
		snprintf(progressStr, sizeof(progressStr), "%.1f%%", progress * 100.0f);
		progressText.setString(progressStr);
		progressText.setPosition(barX + barWidth / 2 - progressText.getLocalBounds().width / 2, 
		                         barY + barHeight / 2 - progressText.getLocalBounds().height / 2);
		window.draw(progressText);
		
		startY += 40;
		
		// Status
		sf::Text statusText;
		statusText.setFont(*font);
		statusText.setCharacterSize(20);
		statusText.setFillColor(sf::Color(200, 200, 255));
		statusText.setString("Status: " + status);
		statusText.setPosition(windowWidth / 2 - statusText.getLocalBounds().width / 2, startY);
		window.draw(statusText);
		
		startY += lineHeight;
		
		// ETA display
		if (!eta.empty() && progress < 1.0f) {
			sf::Text etaText;
			etaText.setFont(*font);
			etaText.setCharacterSize(20);
			etaText.setFillColor(sf::Color(150, 255, 150));  // Verde claro
			etaText.setString("Time Remaining: " + eta);
			etaText.setPosition(windowWidth / 2 - etaText.getLocalBounds().width / 2, startY);
			window.draw(etaText);
			startY += lineHeight;
		}
		
		startY += 10;
		
		// Fitness stats con all-time high
		sf::Text fitnessText;
		fitnessText.setFont(*font);
		fitnessText.setCharacterSize(20);
		fitnessText.setFillColor(sf::Color(100, 255, 200));
		char fitnessStr[128];
		snprintf(fitnessStr, sizeof(fitnessStr), 
		         "Avg Fitness: %.3f  |  Best (Gen): %.3f  |  ATH: %.3f", 
		         avgFitness, bestFitness, allTimeBestFitness);
		fitnessText.setString(fitnessStr);
		fitnessText.setPosition(windowWidth / 2 - fitnessText.getLocalBounds().width / 2, startY);
		window.draw(fitnessText);
		
		startY += lineHeight + 15;
		
		// All-time champion section
		if (champion) {
			sf::Text championTitle;
			championTitle.setFont(*font);
			championTitle.setCharacterSize(24);
			championTitle.setFillColor(sf::Color(255, 215, 0)); // Gold
			championTitle.setString("ALL-TIME CHAMPION");
			championTitle.setPosition(windowWidth / 2 - championTitle.getLocalBounds().width / 2, startY);
			window.draw(championTitle);
			
			startY += 35;
			
			sf::Text championStats;
			championStats.setFont(*font);
			championStats.setCharacterSize(20);
			championStats.setFillColor(sf::Color(255, 255, 100));
			char champStr[128];
			snprintf(champStr, sizeof(champStr), 
			         "ID: %s  |  Fitness: %.3f  |  Record: %d-%d (%.1f%%)",
			         champion->id.c_str(), champion->fitness, 
			         champion->wins, champion->losses, 
			         champion->winRate * 100.0f);
			championStats.setString(champStr);
			championStats.setPosition(windowWidth / 2 - championStats.getLocalBounds().width / 2, startY);
			window.draw(championStats);
		}
	}
	
	// Render top individuals ranking  
	template<typename T>
	void renderTopIndividuals(sf::RenderWindow& window, 
	                          const vector<T*>& topIndividuals,
	                          int displayCount = 5,
	                          const string& allTimeChampionId = "") {
		float startY = 280;
		float lineHeight = 32;
		
		// Title
		sf::Text title;
		title.setFont(*font);
		title.setCharacterSize(28);
		title.setFillColor(sf::Color(255, 200, 100));
		title.setString("Top Performers");
		title.setPosition(windowWidth / 2 - title.getLocalBounds().width / 2, startY);
		window.draw(title);
		
		startY += 40;
		
		// Column headers
		sf::Text header;
		header.setFont(*font);
		header.setCharacterSize(18);
		header.setFillColor(sf::Color(200, 200, 200));
		header.setString("Rank    ID     Fitness    W-L    Win Rate");
		header.setPosition(windowWidth / 2 - 250, startY);
		window.draw(header);
		
		startY += 25;
		
		// Draw each individual
		for (int i = 0; i < min(displayCount, (int)topIndividuals.size()); i++) {
			auto* ind = topIndividuals[i];
			if (!ind) continue;
			
			sf::Text rankText;
			rankText.setFont(*font);
			rankText.setCharacterSize(20);
			
			// Check if this is the all-time champion
			bool isAllTimeChampion = (!allTimeChampionId.empty() && ind->id == allTimeChampionId);
			
			// Color based on rank, with special treatment for all-time champion
			if (isAllTimeChampion) {
				// All-time champion gets bright gold color
				rankText.setFillColor(sf::Color(255, 255, 100)); // Bright gold
			} else if (i == 0) {
				rankText.setFillColor(sf::Color(255, 215, 0)); // Gold
			} else if (i == 1) {
				rankText.setFillColor(sf::Color(192, 192, 192)); // Silver
			} else if (i == 2) {
				rankText.setFillColor(sf::Color(205, 127, 50)); // Bronze
			} else {
				rankText.setFillColor(sf::Color::White);
			}
			
			// Build ID string with indicator for all-time champion
			string idDisplay = ind->id;
			if (isAllTimeChampion) {
				idDisplay += " ★";  // Star indicator for all-time champion
			}
			
			char rankStr[128];
			snprintf(rankStr, sizeof(rankStr), "#%-2d   %-6s    %.3f    %2d-%2d    %.1f%%",
			         i + 1, idDisplay.c_str(), ind->fitness, 
			         ind->wins, ind->losses, ind->winRate * 100.0f);
			rankText.setString(rankStr);
			rankText.setPosition(windowWidth / 2 - 250, startY);
			window.draw(rankText);
			
			startY += lineHeight;
		}
	}
};
