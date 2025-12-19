#pragma once

#include <SFML/Graphics.hpp>
#include "QLearningAgent.h"
#include "Ball.h"
#include <vector>

using namespace std;

enum class DifficultyLevel {
    Easy,
    Medium,
    Hard
};

class AIPlayer {
private:
    sf::RectangleShape shape;
    QLearningAgent agent;
    DifficultyLevel difficulty;
    float speed;
    float reactionDelay;
    float reactionTimer;
    int score;
    
    vector<float> lastState;
    int lastAction;
    bool trainingEnabled;
    
    // Normalize values for neural network input
    vector<float> normalizeState(float ballX, float ballY, float ballVx, float ballVy,
                                  float paddleY, float windowHeight) {
        // Normalize to [0, 1] range
        vector<float> state(6);
        state[0] = ballX / 1280.0f;  // Normalized ball X position
        state[1] = ballY / 720.0f;  // Normalized ball Y position
        state[2] = (ballVx + 1.0f) / 2.0f;  // Normalized velocity X (assuming -1 to 1)
        state[3] = (ballVy + 1.0f) / 2.0f;  // Normalized velocity Y
        state[4] = paddleY / 720.0f;  // Normalized paddle Y position
        state[5] = (paddleY + shape.getSize().y / 2.0f) / 720.0f;  // Normalized paddle center
        
        return state;
    }
    
public:
    AIPlayer(float x, float y, float width, float height, DifficultyLevel diff = DifficultyLevel::Medium)
        : difficulty(diff), score(0), lastAction(1), trainingEnabled(true) {
        shape.setPosition(x, y);
        shape.setSize(sf::Vector2f(width, height));
        shape.setFillColor(sf::Color(65, 65, 255));
        
        reactionTimer = 0.0f;
        setDifficulty(diff);
    }
    
    // Constructor with custom hyperparameters
    AIPlayer(float x, float y, float width, float height, 
             float learningRate, float epsilonDecay, int hiddenSize, 
             float discountFactor, int batchSize, DifficultyLevel diff = DifficultyLevel::Medium)
        : agent(6, 3, hiddenSize, learningRate, epsilonDecay, discountFactor, batchSize),
          difficulty(diff), score(0), lastAction(1), trainingEnabled(true) {
        shape.setPosition(x, y);
        shape.setSize(sf::Vector2f(width, height));
        shape.setFillColor(sf::Color(65, 65, 255));
        
        reactionTimer = 0.0f;
        setDifficulty(diff);
    }
    
    void update(const Ball& ball, float dt, float windowHeight, float windowWidth) {
        // Get current state
        sf::Vector2f ballPos = ball.sprite.getPosition();
        sf::Vector2f ballDir = ball.getDir();
        float paddleY = shape.getPosition().y;
        
        vector<float> currentState = normalizeState(
            ballPos.x, ballPos.y,
            ballDir.x, ballDir.y,
            paddleY, windowHeight
        );
        
        // Update reaction timer
        reactionTimer += dt;
        
        // Decide action if reaction delay has passed
        if (reactionTimer >= reactionDelay) {
            int action = agent.selectAction(currentState);
            lastAction = action;
            lastState = currentState;
            reactionTimer = 0.0f;
        }
        
        // Execute action
        executeAction(lastAction, dt, windowHeight);
        
        // Record experience periodically (every frame when state changes)
        if (trainingEnabled && !lastState.empty()) {
            // Record experience with current state as next state
            recordExperience(false, false, false, false, currentState);
        }
        
        // Update agent training
        if (trainingEnabled) {
            agent.update();
        }
    }
    
    void executeAction(int action, float dt, float windowHeight) {
        sf::Vector2f pos = shape.getPosition();
        sf::Vector2f size = shape.getSize();
        
        // Action: 0 = up, 1 = stay, 2 = down
        if (action == 0) {
            pos.y -= speed * dt;
            if (pos.y < 0) {
                pos.y = 0;
            }
        } else if (action == 2) {
            pos.y += speed * dt;
            if (pos.y + size.y > windowHeight) {
                pos.y = windowHeight - size.y;
            }
        }
        // action == 1 means stay, do nothing
        
        shape.setPosition(pos);
    }
    
    // Record experience for training
    void recordExperience(bool hitBall, bool missedBall, bool scored, bool opponentScored, 
                         const vector<float>& currentState) {
        if (!trainingEnabled || lastState.empty()) return;
        
        float reward = agent.calculateReward(
            lastState, currentState,
            hitBall, missedBall, scored, opponentScored
        );
        
        Experience exp(lastState, lastAction, reward, currentState, false);
        agent.remember(exp);
    }
    
    // Record final experience when point is scored
    void recordPointExperience(bool wonPoint, bool gameOver = false) {
        if (!trainingEnabled || lastState.empty()) return;
        
        vector<float> nextState = lastState;
        float reward = wonPoint ? 5.0f : -10.0f;  // Castigo significativo por recibir gol
        
        // Larger reward/penalty if game is over
        if (gameOver) {
            reward = wonPoint ? 50.0f : -100.0f;  // Castigo muy severo por perder partida
        }
        
        Experience exp(lastState, lastAction, reward, nextState, gameOver);
        agent.remember(exp);
    }
    
    void setDifficulty(DifficultyLevel level) {
        difficulty = level;
        
        // Guardar el epsilon actual si está entrenando
        float currentEpsilon = agent.getEpsilon();
        bool wasTraining = trainingEnabled;
        
        switch (level) {
            case DifficultyLevel::Easy:
                speed = 200.0f;
                reactionDelay = 0.2f;
                // Solo sobrescribir epsilon si no está entrenando
                if (!wasTraining) {
                    agent.setEpsilon(0.3f);
                }
                break;
            case DifficultyLevel::Medium:
                speed = 300.0f;
                reactionDelay = 0.1f;
                if (!wasTraining) {
                    agent.setEpsilon(0.1f);
                }
                break;
            case DifficultyLevel::Hard:
                speed = 400.0f;
                reactionDelay = 0.0f;
                if (!wasTraining) {
                    agent.setEpsilon(0.0f);
                }
                break;
        }
        
        // Si estaba entrenando, restaurar el epsilon para que continúe decayendo
        if (wasTraining && currentEpsilon > 0.1f) {
            agent.setEpsilon(currentEpsilon);
        }
    }
    
    // Llamar después de cada partida para entrenar
    void trainAfterEpisode() {
        if (trainingEnabled) {
            agent.trainAfterEpisode();
        }
    }
    
    // Registrar resultado de una partida
    void recordGameResult(bool won) {
        agent.recordGameResult(won);
    }
    
    // Obtener estadísticas del agente
    int getWins() const {
        return agent.getWins();
    }
    
    int getTotalGames() const {
        return agent.getTotalGames();
    }
    
    DifficultyLevel getDifficulty() const {
        return difficulty;
    }
    
    void setTrainingEnabled(bool enabled) {
        trainingEnabled = enabled;
        agent.setTrainingEnabled(enabled);
    }
    
    bool isTrainingEnabled() const {
        return trainingEnabled;
    }
    
    void render(sf::RenderWindow& window) {
        window.draw(shape);
    }
    
    sf::FloatRect getBounds() const {
        return shape.getGlobalBounds();
    }
    
    int getScore() const {
        return score;
    }
    
    void setScore(int s) {
        score = s;
    }
    
    void incrementScore() {
        score++;
    }
    
    sf::Vector2f getPosition() const {
        return shape.getPosition();
    }
    
    void setPosition(float x, float y) {
        shape.setPosition(x, y);
    }
    
    sf::Vector2f getSize() const {
        return shape.getSize();
    }
    
    QLearningAgent& getAgent() {
        return agent;
    }
    
    const QLearningAgent& getAgent() const {
        return agent;
    }
    
    // Get current state for external use
    vector<float> getCurrentState(const Ball& ball, float windowHeight) {
        sf::Vector2f ballPos = ball.sprite.getPosition();
        sf::Vector2f ballDir = ball.getDir();
        float paddleY = shape.getPosition().y;
        
        return normalizeState(
            ballPos.x, ballPos.y,
            ballDir.x, ballDir.y,
            paddleY, windowHeight
        );
    }
    
    // Clone the AI player with a new position
    AIPlayer* clone(float x, float y) const {
        AIPlayer* cloned = new AIPlayer(
            x, y, 
            shape.getSize().x, shape.getSize().y,
            agent.getLearningRate(),
            agent.getEpsilonDecay(),
            agent.getHiddenLayerSize(),
            agent.getDiscountFactor(),
            agent.getBatchSize(),
            difficulty
        );
        
        // Copy agent state
        agent.copyTo(cloned->agent);
        cloned->trainingEnabled = this->trainingEnabled;
        
        return cloned;
    }
    
    // Get hyperparameters for genetic algorithm
    float getLearningRate() const { return agent.getLearningRate(); }
    float getEpsilonDecay() const { return agent.getEpsilonDecay(); }
    int getHiddenLayerSize() const { return agent.getHiddenLayerSize(); }
    float getDiscountFactor() const { return agent.getDiscountFactor(); }
    int getBatchSize() const { return agent.getBatchSize(); }
};
