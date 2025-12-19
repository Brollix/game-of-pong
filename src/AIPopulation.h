#pragma once

#include "AIPlayer.h"
#include <vector>
#include <random>
#include <algorithm>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

// Genetic parameters for an individual
struct GeneticParams {
    float learningRate;      // 0.001 - 0.05
    float epsilonDecay;      // 0.95 - 0.995
    int hiddenLayerSize;     // 8 - 24
    float discountFactor;    // 0.90 - 0.99
    int batchSize;           // 16 - 64
    
    GeneticParams() 
        : learningRate(0.01f), epsilonDecay(0.98f), hiddenLayerSize(12),
          discountFactor(0.95f), batchSize(32) {}
    
    GeneticParams(float lr, float ed, int hls, float df, int bs)
        : learningRate(lr), epsilonDecay(ed), hiddenLayerSize(hls),
          discountFactor(df), batchSize(bs) {}
};

// Individual in the population
struct AIIndividual {
    string id;  // Alphanumeric ID (4 digits: 0-9, A-Z)
    int generation;
    GeneticParams genes;
    AIPlayer* player;
    
    // Fitness metrics
    float fitness;
    int wins;
    int losses;
    int totalMatches;
    float winRate;
    
    AIIndividual() 
        : id("0000"), generation(0), player(nullptr), fitness(0.0f), 
          wins(0), losses(0), totalMatches(0), winRate(0.0f) {}
    
    AIIndividual(const string& individualId, int gen, const GeneticParams& params, float x, float y)
        : id(individualId), generation(gen), genes(params), fitness(0.0f),
          wins(0), losses(0), totalMatches(0), winRate(0.0f) {
        // Create AI player with genetic parameters
        player = new AIPlayer(x, y, 15, 100,
                            params.learningRate,
                            params.epsilonDecay,
                            params.hiddenLayerSize,
                            params.discountFactor,
                            params.batchSize,
                            DifficultyLevel::Hard);
    }
    
    ~AIIndividual() {
        if (player) {
            delete player;
            player = nullptr;
        }
    }
    
    // Copy constructor
    AIIndividual(const AIIndividual& other) 
        : id(other.id), generation(other.generation), genes(other.genes),
          fitness(other.fitness), wins(other.wins), losses(other.losses),
          totalMatches(other.totalMatches), winRate(other.winRate) {
        if (other.player) {
            player = other.player->clone(other.player->getPosition().x, 
                                        other.player->getPosition().y);
        } else {
            player = nullptr;
        }
    }
    
    // Assignment operator
    AIIndividual& operator=(const AIIndividual& other) {
        if (this != &other) {
            if (player) delete player;
            
            id = other.id;
            generation = other.generation;
            genes = other.genes;
            fitness = other.fitness;
            wins = other.wins;
            losses = other.losses;
            totalMatches = other.totalMatches;
            winRate = other.winRate;
            
            if (other.player) {
                player = other.player->clone(other.player->getPosition().x,
                                            other.player->getPosition().y);
            } else {
                player = nullptr;
            }
        }
        return *this;
    }
    
    void recordMatch(bool won) {
        totalMatches++;
        if (won) {
            wins++;
        } else {
            losses++;
        }
        winRate = totalMatches > 0 ? (float)wins / (float)totalMatches : 0.0f;
    }
    
    void calculateFitness() {
        // Fitness combines win rate (70%) and agent's internal fitness (30%)
        float agentFitness = player ? player->getAgent().getCurrentFitness() : 0.0f;
        fitness = (winRate * 0.7f) + (agentFitness * 0.3f);
    }
    
    void resetStats() {
        wins = 0;
        losses = 0;
        totalMatches = 0;
        winRate = 0.0f;
        fitness = 0.0f;
    }
};

// Population management
class AIPopulation {
private:
    vector<AIIndividual> individuals;
    int populationSize;
    int currentGeneration;
    int nextIdCounter;  // Counter for generating unique IDs
    
    random_device rd;
    mt19937 gen;
    uniform_real_distribution<float> dist;
    
    // Generate alphanumeric ID (4 digits: 0-9, A-Z)
    string generateAlphanumericId() {
        const string chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        uniform_int_distribution<int> charDist(0, chars.size() - 1);
        
        string id = "";
        for (int i = 0; i < 4; i++) {
            id += chars[charDist(gen)];
        }
        return id;
    }
    
public:
    // Generate random genetic parameters (public for TournamentManager)
    GeneticParams randomGenes() {
        uniform_real_distribution<float> lrDist(0.001f, 0.05f);
        uniform_real_distribution<float> edDist(0.95f, 0.995f);
        uniform_int_distribution<int> hlsDist(8, 24);
        uniform_real_distribution<float> dfDist(0.90f, 0.99f);
        uniform_int_distribution<int> bsDist(16, 64);
        
        return GeneticParams(
            lrDist(gen),
            edDist(gen),
            hlsDist(gen),
            dfDist(gen),
            bsDist(gen)
        );
    }
    
    // Crossover two parents
    GeneticParams crossover(const GeneticParams& parent1, const GeneticParams& parent2) {
        uniform_int_distribution<int> coinFlip(0, 1);
        
        return GeneticParams(
            coinFlip(gen) ? parent1.learningRate : parent2.learningRate,
            coinFlip(gen) ? parent1.epsilonDecay : parent2.epsilonDecay,
            coinFlip(gen) ? parent1.hiddenLayerSize : parent2.hiddenLayerSize,
            coinFlip(gen) ? parent1.discountFactor : parent2.discountFactor,
            coinFlip(gen) ? parent1.batchSize : parent2.batchSize
        );
    }
    
    // Mutate genes
    GeneticParams mutate(const GeneticParams& genes, float mutationRate = 0.1f) {
        uniform_real_distribution<float> mutationChance(0.0f, 1.0f);
        uniform_real_distribution<float> mutationAmount(0.8f, 1.2f);
        
        GeneticParams mutated = genes;
        
        if (mutationChance(gen) < mutationRate) {
            mutated.learningRate = max(0.001f, min(0.05f, genes.learningRate * mutationAmount(gen)));
        }
        if (mutationChance(gen) < mutationRate) {
            mutated.epsilonDecay = max(0.95f, min(0.995f, genes.epsilonDecay * mutationAmount(gen)));
        }
        if (mutationChance(gen) < mutationRate) {
            uniform_int_distribution<int> hlsMutation(-4, 4);
            mutated.hiddenLayerSize = max(8, min(24, genes.hiddenLayerSize + hlsMutation(gen)));
        }
        if (mutationChance(gen) < mutationRate) {
            mutated.discountFactor = max(0.90f, min(0.99f, genes.discountFactor * mutationAmount(gen)));
        }
        if (mutationChance(gen) < mutationRate) {
            uniform_int_distribution<int> bsMutation(-8, 8);
            mutated.batchSize = max(16, min(64, genes.batchSize + bsMutation(gen)));
        }
        
        return mutated;
    }
    
    AIPopulation(int size = 16) 
        : populationSize(size), currentGeneration(0), nextIdCounter(0), gen(rd()), dist(0.0f, 1.0f) {
    }
    
    ~AIPopulation() {
        individuals.clear();
    }
    
    // Initialize population with random genes
    void initialize(float x, float y) {
        individuals.clear();
        nextIdCounter = 0;
        
        for (int i = 0; i < populationSize; i++) {
            GeneticParams genes = randomGenes();
            string id = generateAlphanumericId();
            individuals.emplace_back(id, currentGeneration, genes, x, y);
        }
        
        cout << "Population initialized with " << populationSize << " individuals" << endl;
    }
    
    // Initialize population based on a base individual with mutations
    void initializeFromBase(const AIIndividual& baseIndividual, float x, float y, float mutationRate = 0.15f) {
        individuals.clear();
        nextIdCounter = 0;
        
        // First individual is the base (will be added by TournamentManager with preserved stats)
        // Fill rest with mutations of the base
        for (int i = 0; i < populationSize; i++) {
            GeneticParams mutatedGenes = mutate(baseIndividual.genes, mutationRate);
            string id = generateAlphanumericId();
            individuals.emplace_back(id, currentGeneration, mutatedGenes, x, y);
        }
        
        cout << "Population initialized with " << populationSize 
             << " individuals based on base individual " << baseIndividual.id << endl;
    }
    
    // Calculate fitness for all individuals
    void calculateAllFitness() {
        for (auto& individual : individuals) {
            individual.calculateFitness();
        }
    }
    
    // Sort by fitness (descending)
    void sortByFitness() {
        sort(individuals.begin(), individuals.end(),
             [](const AIIndividual& a, const AIIndividual& b) {
                 return a.fitness > b.fitness;
             });
    }
    
    // Evolve to next generation
    void evolveNextGeneration(float elitePercent = 0.25f, float mutationRate = 0.1f) {
        calculateAllFitness();
        sortByFitness();
        
        int eliteCount = max(1, (int)(populationSize * elitePercent));
        
        // Store elite individuals
        vector<AIIndividual> nextGen;
        for (int i = 0; i < eliteCount; i++) {
            AIIndividual elite = individuals[i];
            elite.generation = currentGeneration + 1;
            elite.resetStats();
            nextGen.push_back(elite);
        }
        
        // Generate offspring to fill the rest
        while (nextGen.size() < populationSize) {
            // Select two random parents from elite
            uniform_int_distribution<int> parentDist(0, eliteCount - 1);
            int parent1Idx = parentDist(gen);
            int parent2Idx = parentDist(gen);
            
            // Crossover and mutate
            GeneticParams childGenes = crossover(individuals[parent1Idx].genes, 
                                                 individuals[parent2Idx].genes);
            childGenes = mutate(childGenes, mutationRate);
            
            // Create new individual
            float x = individuals[0].player->getPosition().x;
            float y = individuals[0].player->getPosition().y;
            string id = generateAlphanumericId();
            nextGen.emplace_back(id, currentGeneration + 1, childGenes, x, y);
        }
        
        individuals = move(nextGen);
        currentGeneration++;
        
        cout << "Generation " << currentGeneration << " created with " 
             << eliteCount << " elite individuals" << endl;
    }
    
    // Get individuals
    vector<AIIndividual>& getIndividuals() { return individuals; }
    const vector<AIIndividual>& getIndividuals() const { return individuals; }
    
    // Get individual by index
    AIIndividual& getIndividual(int index) { return individuals[index]; }
    const AIIndividual& getIndividual(int index) const { return individuals[index]; }
    
    // Get best individual
    AIIndividual& getBest() {
        sortByFitness();
        return individuals[0];
    }
    
    // Get population stats
    int getSize() const { return individuals.size(); }
    int getGeneration() const { return currentGeneration; }
    
    // Generate new alphanumeric ID (for TournamentManager)
    string generateNewId() { 
        nextIdCounter++;
        return generateAlphanumericId(); 
    }
    void setNextIdCounter(int counter) { nextIdCounter = counter; }
    int getNextIdCounter() const { return nextIdCounter; }
    
    float getAverageFitness() const {
        if (individuals.empty()) return 0.0f;
        float sum = 0.0f;
        for (const auto& ind : individuals) {
            sum += ind.fitness;
        }
        return sum / individuals.size();
    }
    
    float getBestFitness() const {
        if (individuals.empty()) return 0.0f;
        float best = 0.0f;
        for (const auto& ind : individuals) {
            if (ind.fitness > best) best = ind.fitness;
        }
        return best;
    }
    
    // Save population to file
    bool savePopulation(const string& filename) const {
        ofstream file(filename, ios::binary);
        if (!file.is_open()) {
            cerr << "Failed to save population to " << filename << endl;
            return false;
        }
        
        // Write header
        file.write(reinterpret_cast<const char*>(&populationSize), sizeof(populationSize));
        file.write(reinterpret_cast<const char*>(&currentGeneration), sizeof(currentGeneration));
        file.write(reinterpret_cast<const char*>(&nextIdCounter), sizeof(nextIdCounter));
        
        // Write individuals
        for (const auto& ind : individuals) {
            // Write ID as string (4 characters)
            int idLen = ind.id.length();
            file.write(reinterpret_cast<const char*>(&idLen), sizeof(idLen));
            file.write(ind.id.c_str(), idLen);
            file.write(reinterpret_cast<const char*>(&ind.generation), sizeof(ind.generation));
            file.write(reinterpret_cast<const char*>(&ind.genes), sizeof(ind.genes));
            file.write(reinterpret_cast<const char*>(&ind.fitness), sizeof(ind.fitness));
            file.write(reinterpret_cast<const char*>(&ind.wins), sizeof(ind.wins));
            file.write(reinterpret_cast<const char*>(&ind.losses), sizeof(ind.losses));
            file.write(reinterpret_cast<const char*>(&ind.totalMatches), sizeof(ind.totalMatches));
            file.write(reinterpret_cast<const char*>(&ind.winRate), sizeof(ind.winRate));
        }
        
        file.close();
        cout << "Population saved to " << filename << endl;
        return true;
    }
    
    // Load population from file
    bool loadPopulation(const string& filename, float x, float y) {
        ifstream file(filename, ios::binary);
        if (!file.is_open()) {
            cerr << "Failed to load population from " << filename << endl;
            return false;
        }
        
        // Read header
        file.read(reinterpret_cast<char*>(&populationSize), sizeof(populationSize));
        file.read(reinterpret_cast<char*>(&currentGeneration), sizeof(currentGeneration));
        file.read(reinterpret_cast<char*>(&nextIdCounter), sizeof(nextIdCounter));
        
        individuals.clear();
        
        // Read individuals
        for (int i = 0; i < populationSize; i++) {
            string id;
            int generation;
            GeneticParams genes;
            float fitness, winRate;
            int wins, losses, totalMatches;
            
            // Read ID as string
            int idLen;
            file.read(reinterpret_cast<char*>(&idLen), sizeof(idLen));
            id.resize(idLen);
            file.read(&id[0], idLen);
            file.read(reinterpret_cast<char*>(&generation), sizeof(generation));
            file.read(reinterpret_cast<char*>(&genes), sizeof(genes));
            file.read(reinterpret_cast<char*>(&fitness), sizeof(fitness));
            file.read(reinterpret_cast<char*>(&wins), sizeof(wins));
            file.read(reinterpret_cast<char*>(&losses), sizeof(losses));
            file.read(reinterpret_cast<char*>(&totalMatches), sizeof(totalMatches));
            file.read(reinterpret_cast<char*>(&winRate), sizeof(winRate));
            
            individuals.emplace_back(id, generation, genes, x, y);
            individuals.back().fitness = fitness;
            individuals.back().wins = wins;
            individuals.back().losses = losses;
            individuals.back().totalMatches = totalMatches;
            individuals.back().winRate = winRate;
        }
        
        file.close();
        cout << "Population loaded from " << filename << endl;
        return true;
    }
};

