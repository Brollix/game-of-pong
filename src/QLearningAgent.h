#pragma once

#include "NeuralNetwork.h"
#include <deque>
#include <random>
#include <algorithm>
#include <cmath>

using namespace std;

struct Experience {
    vector<float> state;
    int action;
    float reward;
    vector<float> nextState;
    bool done;
    
    Experience() : action(0), reward(0.0f), done(false) {}
    
    Experience(const vector<float>& s, int a, float r, const vector<float>& ns, bool d)
        : state(s), action(a), reward(r), nextState(ns), done(d) {}
};

class QLearningAgent {
private:
    NeuralNetwork qNetwork;
    deque<Experience> replayBuffer;
    int maxBufferSize;
    
    float epsilon;
    float epsilonMin;
    float epsilonDecay;
    float learningRate;
    float discountFactor;
    int batchSize;
    int updateFrequency;
    int frameCount;
    int episodeCount;  // Contador de episodios (partidas) completadas
    int wins;  // Número de victorias
    int totalGames;  // Total de partidas jugadas
    float bestWinRate;  // Mejor win rate alcanzado
    deque<bool> recentResults;  // Resultados recientes para calcular fitness
    int recentWindowSize;  // Tamaño de la ventana para fitness
    float bestFitness;  // Mejor fitness alcanzado
    
    random_device rd;
    mt19937 gen;
    uniform_real_distribution<float> dist;
    
    // Sample random experiences from replay buffer
    vector<Experience> sampleBatch() {
        vector<Experience> batch;
        int sampleSize = min(batchSize, (int)replayBuffer.size());
        
        for (int i = 0; i < sampleSize; i++) {
            uniform_int_distribution<int> indexDist(0, replayBuffer.size() - 1);
            batch.push_back(replayBuffer[indexDist(gen)]);
        }
        
        return batch;
    }
    
public:
    QLearningAgent(int inputSize = 6, int outputSize = 3, int hiddenSize = 12)
        : qNetwork(inputSize, hiddenSize, outputSize),
          maxBufferSize(10000),
          epsilon(1.0f),
          epsilonMin(0.1f),
          epsilonDecay(0.98f),  // Decay más rápido (0.98 en lugar de 0.995)
          learningRate(0.01f),  // Learning rate más alto (0.01 en lugar de 0.001)
          discountFactor(0.95f),
          batchSize(32),
          updateFrequency(2),  // Entrenar cada 2 frames en lugar de 4
          frameCount(0),
          episodeCount(0),
          wins(0),
          totalGames(0),
          bestWinRate(0.0f),
          recentWindowSize(10),  // Usar últimas 10 partidas para fitness
          bestFitness(0.0f),
          gen(rd()),
          dist(0.0f, 1.0f) {
    }
    
    // Constructor with custom hyperparameters
    QLearningAgent(int inputSize, int outputSize, int hiddenSize, 
                   float lr, float epsilonDec, float discount, int batch)
        : qNetwork(inputSize, hiddenSize, outputSize),
          maxBufferSize(10000),
          epsilon(1.0f),
          epsilonMin(0.1f),
          epsilonDecay(epsilonDec),
          learningRate(lr),
          discountFactor(discount),
          batchSize(batch),
          updateFrequency(2),
          frameCount(0),
          episodeCount(0),
          wins(0),
          totalGames(0),
          bestWinRate(0.0f),
          recentWindowSize(10),
          bestFitness(0.0f),
          gen(rd()),
          dist(0.0f, 1.0f) {
    }
    
    // Select action using epsilon-greedy policy
    int selectAction(const vector<float>& state) {
        uniform_real_distribution<float> randDist(0.0f, 1.0f);
        
        if (randDist(gen) < epsilon) {
            // Explore: random action
            uniform_int_distribution<int> actionDist(0, 2);
            return actionDist(gen);
        } else {
            // Exploit: best action according to Q-network
            vector<float> qValues = qNetwork.getQValues(state);
            return max_element(qValues.begin(), qValues.end()) - qValues.begin();
        }
    }
    
    // Store experience in replay buffer
    void remember(const Experience& exp) {
        replayBuffer.push_back(exp);
        if (replayBuffer.size() > maxBufferSize) {
            replayBuffer.pop_front();
        }
    }
    
    // Train the network using experience replay
    void train(int numBatches = 1) {
        if (replayBuffer.size() < batchSize) {
            return;
        }
        
        // Entrenar múltiples batches para aprendizaje más efectivo
        for (int batch = 0; batch < numBatches; batch++) {
            vector<Experience> experienceBatch = sampleBatch();
            
            for (const auto& exp : experienceBatch) {
                vector<float> currentQValues = qNetwork.getQValues(exp.state);
                float targetQ = exp.reward;
                
                if (!exp.done) {
                    vector<float> nextQValues = qNetwork.getQValues(exp.nextState);
                    float maxNextQ = *max_element(nextQValues.begin(), nextQValues.end());
                    targetQ += discountFactor * maxNextQ;
                }
                
                // Update Q-value for the action taken
                qNetwork.updateQValue(exp.state, exp.action, targetQ, learningRate);
            }
        }
    }
    
    // Decay epsilon basado en episodios (llamar después de cada partida)
    void decayEpsilonAfterEpisode() {
        episodeCount++;
        if (epsilon > epsilonMin) {
            epsilon *= epsilonDecay;
            // Asegurar que no baje del mínimo
            if (epsilon < epsilonMin) {
                epsilon = epsilonMin;
            }
        }
    }
    
    // Update and train (called each frame)
    void update() {
        frameCount++;
        if (frameCount % updateFrequency == 0 && replayBuffer.size() >= batchSize) {
            train(1);  // Entrenar 1 batch cada updateFrequency frames
        }
    }
    
    // Entrenar después de un episodio (llamar después de cada partida)
    void trainAfterEpisode() {
        if (replayBuffer.size() >= batchSize) {
            // Entrenar múltiples batches después de cada partida
            train(5);  // 5 batches de entrenamiento después de cada partida
            decayEpsilonAfterEpisode();
        }
    }
    
    // Registrar resultado de una partida
    void recordGameResult(bool won) {
        totalGames++;
        if (won) {
            wins++;
        }
        
        // Mantener ventana de resultados recientes
        recentResults.push_back(won);
        if (recentResults.size() > recentWindowSize) {
            recentResults.pop_front();
        }
    }
    
    // Obtener win rate actual
    float getWinRate() const {
        if (totalGames == 0) return 0.0f;
        return static_cast<float>(wins) / static_cast<float>(totalGames);
    }
    
    // Calcular fitness basado en rendimiento reciente y epsilon
    // Fitness = (win rate reciente * 0.7) + ((1.0 - epsilon) * 0.3)
    // Epsilon bajo = mejor (más explotación), así que (1.0 - epsilon) da valor alto
    float calculateFitness() const {
        // Calcular win rate reciente
        float recentWinRate = 0.0f;
        if (recentResults.size() > 0) {
            int recentWins = 0;
            for (bool won : recentResults) {
                if (won) recentWins++;
            }
            recentWinRate = static_cast<float>(recentWins) / static_cast<float>(recentResults.size());
        }
        
        // Fitness combina rendimiento reciente (70%) y nivel de explotación (30%)
        // Epsilon bajo = más explotación = mejor aprendizaje = mayor fitness
        float exploitationScore = 1.0f - epsilon;  // Normalizado: epsilon 1.0 -> 0.0, epsilon 0.1 -> 0.9
        float fitness = (recentWinRate * 0.7f) + (exploitationScore * 0.3f);
        
        return fitness;
    }
    
    // Verificar si hay mejora usando fitness
    bool checkAndUpdateBestFitness() {
        float currentFitness = calculateFitness();
        if (currentFitness > bestFitness) {
            bestFitness = currentFitness;
            return true;  // Hay mejora
        }
        return false;  // No hay mejora
    }
    
    float getBestFitness() const {
        return bestFitness;
    }
    
    float getCurrentFitness() const {
        return calculateFitness();
    }
    
    // Inicializar bestFitness (útil al cargar métricas guardadas)
    void setBestFitness(float fitness) {
        bestFitness = fitness;
    }
    
    // Métodos legacy para compatibilidad (ahora usan fitness)
    bool checkAndUpdateBestWinRate() {
        return checkAndUpdateBestFitness();
    }
    
    float getBestWinRate() const {
        return bestFitness;  // Retornar fitness como "win rate" para compatibilidad
    }
    
    void setBestWinRate(float rate) {
        bestFitness = rate;  // Guardar como fitness
    }
    
    int getWins() const {
        return wins;
    }
    
    int getTotalGames() const {
        return totalGames;
    }
    
    int getEpisodeCount() const {
        return episodeCount;
    }
    
    int getReplayBufferSize() const {
        return replayBuffer.size();
    }
    
    // Calculate reward based on game state
    float calculateReward(const vector<float>& prevState, const vector<float>& currentState,
                         bool hitBall, bool missedBall, bool scored, bool opponentScored) {
        float reward = 0.0f;
        
        if (hitBall) {
            reward += 20.0f;  // Aumentado de 10 a 20
        }
        
        if (missedBall) {
            reward -= 15.0f;  // Castigo significativo por fallar la pelota
        }
        
        if (scored) {
            reward += 10.0f;  // Aumentado de 5 a 10
        }
        
        if (opponentScored) {
            reward -= 15.0f;  // Castigo significativo por recibir gol
        }
        
        // Reward for moving towards ball (mejorado)
        if (prevState.size() >= 6 && currentState.size() >= 6) {
            float ballY = currentState[1] * 720.0f;  // Desnormalizar
            float paddleY = currentState[4] * 720.0f;
            float prevPaddleY = prevState[4] * 720.0f;
            
            float distanceToBall = abs(ballY - paddleY);
            float prevDistance = abs(ballY - prevPaddleY);
            
            // Recompensa por acercarse a la pelota (sin penalizar por alejarse o quedarse quieto)
            if (distanceToBall < prevDistance) {
                float improvement = prevDistance - distanceToBall;
                reward += improvement * 0.1f;  // Recompensa proporcional a la mejora
            }
            // No castigar por alejarse o quedarse quieto
            
            // Recompensa incremental por estar bien posicionado en Y según la posición X de la pelota
            // state[0] = ballX normalizado (0-1), donde 0.0 = lado izquierdo, 1.0 = lado derecho (paddle)
            // state[1] = ballY normalizado (0-1)
            // state[4] = paddleY normalizado (0-1)
            // state[5] = paddleCenter normalizado (0-1)
            
            float ballXNormalized = currentState[0];
            float ballYNormalized = currentState[1];
            float paddleCenterNormalized = currentState[5];
            
            // Calcular distancia en Y entre el centro del paddle y la pelota
            float distanceY = abs(ballYNormalized - paddleCenterNormalized);
            
            // Calcular qué tan bien alineado está el paddle (0.0 = perfecto, 1.0 = muy lejos)
            // Invertir para que valores más bajos de distanceY den mayor alineación
            float alignmentY = 1.0f - min(1.0f, distanceY * 3.0f);  // Multiplicar por 3 para hacer más estricto
            
            // Calcular proximidad en X (0.0 = otro lado, 1.0 = muy cerca del paddle)
            // El paddle está en el lado derecho, así que ballX cerca de 1.0 = más cerca
            float proximityX = ballXNormalized;  // 0.0 a 1.0 directamente
            
            // Recompensa incremental: combina alineación Y y proximidad X
            // - Si la pelota está del otro lado (proximityX bajo) y está alineado: reward pequeño
            // - Si la pelota está cerca (proximityX alto) y está alineado: reward grande
            // - Si no está alineado: reward muy pequeño o negativo
            
            float baseReward = alignmentY * proximityX;
            
            // Escalar la recompensa para que sea más significativa
            // Máximo reward cuando está perfectamente alineado y la pelota está muy cerca
            float incrementalReward = baseReward * 8.0f;
            
            // Penalización pequeña si está muy desalineado cuando la pelota está cerca
            // Esto motiva al agente a mantenerse alineado
            if (proximityX > 0.5f && alignmentY < 0.3f) {
                incrementalReward -= 2.0f * proximityX;  // Penalización cuando está cerca pero mal alineado
            }
            
            reward += incrementalReward;
            
            // Bonus extra cuando está muy bien posicionado (alineado y cerca)
            if (proximityX > 0.8f && alignmentY > 0.7f) {
                reward += 5.0f;  // Bonus significativo por estar en posición óptima
            }
        }
        
        return reward;
    }
    
    // Set epsilon (for difficulty adjustment)
    void setEpsilon(float eps) {
        epsilon = max(epsilonMin, min(1.0f, eps));
    }
    
    float getEpsilon() const {
        return epsilon;
    }
    
    // Enable/disable training
    void setTrainingEnabled(bool enabled) {
        if (!enabled) {
            epsilon = epsilonMin;  // Stop exploration when not training
        } else {
            // Si se habilita el entrenamiento, asegurar que epsilon no esté en mínimo
            if (epsilon <= epsilonMin) {
                epsilon = 1.0f;  // Reiniciar exploración
            }
        }
    }
    
    // Get network for saving/loading
    NeuralNetwork& getNetwork() {
        return qNetwork;
    }
    
    const NeuralNetwork& getNetwork() const {
        return qNetwork;
    }
    
    // Get hyperparameters
    float getLearningRate() const { return learningRate; }
    float getEpsilonDecay() const { return epsilonDecay; }
    float getDiscountFactor() const { return discountFactor; }
    int getBatchSize() const { return batchSize; }
    int getHiddenLayerSize() const { return qNetwork.getLayerSizes()[1]; }
    
    // Set hyperparameters
    void setLearningRate(float lr) { learningRate = lr; }
    void setEpsilonDecay(float decay) { epsilonDecay = decay; }
    void setDiscountFactor(float discount) { discountFactor = discount; }
    void setBatchSize(int batch) { batchSize = batch; }
    
    // Copy agent state to another agent
    void copyTo(QLearningAgent& target) const {
        // Copy network weights
        qNetwork.copyTo(target.qNetwork);
        
        // Copy other important state
        target.epsilon = this->epsilon;
        target.episodeCount = this->episodeCount;
        target.wins = this->wins;
        target.totalGames = this->totalGames;
        target.bestFitness = this->bestFitness;
        target.recentResults = this->recentResults;
    }
};
