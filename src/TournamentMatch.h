#pragma once

#include "AIPlayer.h"
#include "Ball.h"
#include "Collisions.h"
#include <SFML/Graphics.hpp>
#include <iostream>

using namespace std;

// Result of a tournament match
struct MatchResult {
    string player1Id;
    string player2Id;
    int player1Score;
    int player2Score;
    string winnerId;
    int totalFrames;
    float duration;  // in seconds
    
    MatchResult() 
        : player1Id("0000"), player2Id("0000"), player1Score(0), player2Score(0),
          winnerId("0000"), totalFrames(0), duration(0.0f) {}
};

// Tournament match simulator (headless or visual)
class TournamentMatch {
private:
    AIPlayer* player1;
    AIPlayer* player2;
    Ball* ball;
    
    int winScore;
    int windowWidth;
    int windowHeight;
    
    bool visualMode;
    sf::RenderWindow* window;
    
    // Collision tracking
    bool lastBallHitPlayer1;
    bool lastBallHitPlayer2;
    
    // Match state
    int currentFrames;
    float currentTime;
    
public:
    TournamentMatch(int width, int height, int winningScore = 7, bool visual = false)
        : player1(nullptr), player2(nullptr), ball(nullptr),
          winScore(winningScore), windowWidth(width), windowHeight(height),
          visualMode(visual), window(nullptr),
          lastBallHitPlayer1(false), lastBallHitPlayer2(false),
          currentFrames(0), currentTime(0.0f) {
        
        if (visualMode) {
            window = new sf::RenderWindow(sf::VideoMode(width, height), 
                                         "Tournament Match", 
                                         sf::Style::Titlebar | sf::Style::Close);
            window->setVerticalSyncEnabled(false);
            window->setFramerateLimit(60);
        }
    }
    
    ~TournamentMatch() {
        if (window) {
            delete window;
            window = nullptr;
        }
        // Don't delete players - they're owned by AIPopulation
        if (ball) {
            delete ball;
            ball = nullptr;
        }
    }
    
    // Setup match with two AI players
    void setupMatch(AIPlayer* p1, AIPlayer* p2) {
        player1 = p1;
        player2 = p2;
        
        // Position players on opposite sides
        // Player 1 on left, Player 2 on right
        float paddleWidth = player1->getSize().x;
        float paddleHeight = player1->getSize().y;
        float centerY = windowHeight / 2.0f - paddleHeight / 2.0f;
        
        player1->setPosition(35.0f, centerY);  // Left side
        player2->setPosition(windowWidth - 50.0f, centerY);  // Right side
        
        // Reset scores
        player1->setScore(0);
        player2->setScore(0);
        
        // Create ball
        if (ball) delete ball;
        ball = new Ball(windowWidth, windowHeight);
        ball->reset();
        
        lastBallHitPlayer1 = false;
        lastBallHitPlayer2 = false;
        currentFrames = 0;
        currentTime = 0.0f;
    }
    
    // Run match to completion (headless)
    MatchResult runHeadless(const string& player1Id, const string& player2Id, float speedMultiplier = 1.0f) {
        if (!player1 || !player2 || !ball) {
            cerr << "Match not properly set up!" << endl;
            return MatchResult();
        }
        
        // Disable training during tournament matches
        bool p1TrainingWas = player1->isTrainingEnabled();
        bool p2TrainingWas = player2->isTrainingEnabled();
        player1->setTrainingEnabled(false);
        player2->setTrainingEnabled(false);
        
        const float dt = 0.016f * speedMultiplier;  // ~60 FPS base
        const int maxFrames = 100000;  // Prevent infinite loops
        
        while (player1->getScore() < winScore && 
               player2->getScore() < winScore && 
               currentFrames < maxFrames) {
            
            // Update game logic
            updateGameLogic(dt);
            currentTime += dt;
            currentFrames++;
        }
        
        // Restore training state
        player1->setTrainingEnabled(p1TrainingWas);
        player2->setTrainingEnabled(p2TrainingWas);
        
        // Create result
        MatchResult result;
        result.player1Id = player1Id;
        result.player2Id = player2Id;
        result.player1Score = player1->getScore();
        result.player2Score = player2->getScore();
        result.winnerId = (player1->getScore() >= winScore) ? player1Id : player2Id;
        result.totalFrames = currentFrames;
        result.duration = currentTime;
        
        return result;
    }
    
    // Run match with visualization
    MatchResult runVisual(const string& player1Id, const string& player2Id) {
        if (!player1 || !player2 || !ball || !window) {
            cerr << "Match not properly set up for visual mode!" << endl;
            return MatchResult();
        }
        
        // Disable training during tournament matches
        bool p1TrainingWas = player1->isTrainingEnabled();
        bool p2TrainingWas = player2->isTrainingEnabled();
        player1->setTrainingEnabled(false);
        player2->setTrainingEnabled(false);
        
        sf::Clock clock;
        
        while (window->isOpen() && 
               player1->getScore() < winScore && 
               player2->getScore() < winScore) {
            
            float dt = clock.restart().asSeconds();
            if (dt > 0.1f) dt = 0.1f;  // Cap delta time
            
            // Handle events
            sf::Event event;
            while (window->pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window->close();
                }
                if (event.type == sf::Event::KeyPressed && 
                    event.key.code == sf::Keyboard::Escape) {
                    window->close();
                }
            }
            
            // Update
            updateGameLogic(dt);
            currentTime += dt;
            currentFrames++;
            
            // Render
            window->clear(sf::Color::Black);
            player1->render(*window);
            player2->render(*window);
            ball->render(*window);
            
            // Draw score
            // (Note: Could add HUD here if needed)
            
            window->display();
        }
        
        // Restore training state
        player1->setTrainingEnabled(p1TrainingWas);
        player2->setTrainingEnabled(p2TrainingWas);
        
        // Create result
        MatchResult result;
        result.player1Id = player1Id;
        result.player2Id = player2Id;
        result.player1Score = player1->getScore();
        result.player2Score = player2->getScore();
        result.winnerId = (player1->getScore() >= winScore) ? player1Id : player2Id;
        result.totalFrames = currentFrames;
        result.duration = currentTime;
        
        return result;
    }
    
private:
    void updateGameLogic(float dt) {
        // Update players
        player1->update(*ball, dt, windowHeight, windowWidth);
        player2->update(*ball, dt, windowHeight, windowWidth);
        
        // Update ball
        ball->move(dt, windowWidth, windowHeight);
        
        // Check collisions
        bool currentBallHitPlayer1 = Collisions::check(ball->getBounds(), player1->getBounds());
        bool currentBallHitPlayer2 = Collisions::check(ball->getBounds(), player2->getBounds());
        
        // Handle collisions
        if (currentBallHitPlayer1 && !lastBallHitPlayer1) {
            ball->dir.x *= -1;
            vector<float> currentState = player1->getCurrentState(*ball, windowHeight);
            player1->recordExperience(true, false, false, false, currentState);
        }
        
        if (currentBallHitPlayer2 && !lastBallHitPlayer2) {
            ball->dir.x *= -1;
            vector<float> currentState = player2->getCurrentState(*ball, windowHeight);
            player2->recordExperience(true, false, false, false, currentState);
        }
        
        lastBallHitPlayer1 = currentBallHitPlayer1;
        lastBallHitPlayer2 = currentBallHitPlayer2;
        
        // Check for scoring
        int scoreResult = ball->checkScore(windowWidth);
        if (scoreResult == 1) {
            // Left player (player1) scored
            player1->incrementScore();
            bool gameWon = (player1->getScore() >= winScore);
            player1->recordPointExperience(true, gameWon);
            player2->recordPointExperience(false, gameWon);
            ball->reset();
            lastBallHitPlayer1 = false;
            lastBallHitPlayer2 = false;
        } else if (scoreResult == 2) {
            // Right player (player2) scored
            player2->incrementScore();
            bool gameWon = (player2->getScore() >= winScore);
            player2->recordPointExperience(true, gameWon);
            player1->recordPointExperience(false, gameWon);
            ball->reset();
            lastBallHitPlayer1 = false;
            lastBallHitPlayer2 = false;
        }
    }
};

