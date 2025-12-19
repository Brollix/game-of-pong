#pragma once

#include "AIPopulation.h"
#include "TournamentMatch.h"
#include "ModelSaver.h"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <algorithm>

using namespace std;

enum class TournamentMode {
    RoundRobin,    // Everyone plays everyone
    Swiss,         // Swiss system pairing
    Evolutionary   // Generation-based evolution
};

enum class TournamentState {
    Idle,
    Running,
    Paused,
    Completed
};

struct TournamentConfig {
    int populationSize;
    int maxGenerations;
    int pointsPerMatch;
    float speedMultiplier;
    float elitePercent;
    float mutationRate;
    TournamentMode mode;
    
    TournamentConfig()
        : populationSize(16), maxGenerations(50), pointsPerMatch(7),
          speedMultiplier(10.0f), elitePercent(0.25f), mutationRate(0.1f),
          mode(TournamentMode::Evolutionary) {}
};

struct GenerationStats {
    int generation;
    float avgFitness;
    float bestFitness;
    float worstFitness;
    string bestIndividualId;
    float avgWinRate;
    
    GenerationStats()
        : generation(0), avgFitness(0.0f), bestFitness(0.0f), 
          worstFitness(0.0f), bestIndividualId("0000"), avgWinRate(0.0f) {}
};

class TournamentManager {
private:
    AIPopulation* population;
    TournamentMatch* match;
    TournamentConfig config;
    TournamentState state;
    
    int windowWidth;
    int windowHeight;
    
    int currentGeneration;
    int currentMatch;
    int totalMatches;
    
    vector<GenerationStats> statsHistory;
    
    // Progress tracking
    float progress;  // 0.0 to 1.0
    string statusMessage;
    
    // Cache of top individuals for display (before evolution resets stats)
    vector<AIIndividual> cachedTopIndividuals;
    
    // All-time tracking
    float allTimeBestFitness;
    AIIndividual allTimeBestIndividual;
    bool hasAllTimeBest;
    
    // Previous tournament winner tracking
    AIIndividual previousTournamentWinner;
    bool hasPreviousWinner;
    
    // ETA tracking
    sf::Clock etaClock;
    float totalElapsedTime;
    float lastGenerationStartTime;
    vector<float> generationTimes;
    float estimatedTimeRemaining;
    
public:
    TournamentManager(int width, int height)
        : population(nullptr), match(nullptr), state(TournamentState::Idle),
          windowWidth(width), windowHeight(height),
          currentGeneration(0), currentMatch(0), totalMatches(0),
          progress(0.0f), statusMessage("Ready"),
          allTimeBestFitness(0.0f), hasAllTimeBest(false),
          hasPreviousWinner(false),
          totalElapsedTime(0.0f), lastGenerationStartTime(0.0f), 
          estimatedTimeRemaining(0.0f) {
        
        population = new AIPopulation(config.populationSize);
        match = new TournamentMatch(width, height, config.pointsPerMatch, false);
    }
    
    ~TournamentManager() {
        if (population) delete population;
        if (match) delete match;
    }
    
    // Initialize tournament
    void initialize() {
        float x = windowWidth - 50;
        float y = windowHeight / 2;
        
        // Try to load previous tournament winner
        if (loadTournamentWinner(x, y)) {
            // Previous winner exists - create population based on mutations of winner
            population->initializeFromBase(previousTournamentWinner, x, y, config.mutationRate);
            
            // Add previous winner to population with preserved stats
            auto& individuals = population->getIndividuals();
            // Replace first individual with previous winner (preserving stats)
            AIIndividual winnerCopy = previousTournamentWinner;
            winnerCopy.generation = 0;  // Reset generation for new tournament
            individuals[0] = winnerCopy;
            
            cout << "Tournament initialized with previous winner " << previousTournamentWinner.id 
                 << " (Fitness: " << previousTournamentWinner.fitness << ")" << endl;
        } else {
            // No previous winner - try to load top 5 from previous tournament
            if (!loadTop5ForPersistence(x, y)) {
                // If no previous tournament, initialize with random genes
                population->initialize(x, y);
            }
        }
        
        currentGeneration = 0;
        currentMatch = 0;
        statsHistory.clear();
        allTimeBestFitness = 0.0f;
        hasAllTimeBest = false;
        // hasPreviousWinner is set by loadTournamentWinner() if winner exists
        if (!hasPreviousWinner) {
            // Ensure it's false if no winner was loaded
            hasPreviousWinner = false;
        }
        state = TournamentState::Idle;
        statusMessage = "Tournament initialized";
    }
    
    // Set configuration
    void setConfig(const TournamentConfig& cfg) {
        config = cfg;
        if (population) {
            delete population;
            population = new AIPopulation(config.populationSize);
        }
        if (match) {
            delete match;
            match = new TournamentMatch(windowWidth, windowHeight, 
                                       config.pointsPerMatch, false);
        }
    }
    
    // Start tournament
    void start() {
        if (state == TournamentState::Running) {
            return;
        }
        
        if (population->getSize() == 0) {
            initialize();
        }
        
        // Initialize ETA tracking
        etaClock.restart();
        totalElapsedTime = 0.0f;
        generationTimes.clear();
        estimatedTimeRemaining = 0.0f;
        
        state = TournamentState::Running;
        statusMessage = "Tournament started";
    }
    
    // Pause tournament
    void pause() {
        if (state == TournamentState::Running) {
            state = TournamentState::Paused;
            statusMessage = "Tournament paused";
        }
    }
    
    // Resume tournament
    void resume() {
        if (state == TournamentState::Paused) {
            state = TournamentState::Running;
            statusMessage = "Tournament resumed";
        }
    }
    
    // Stop tournament
    void stop() {
        state = TournamentState::Idle;
        statusMessage = "Tournament stopped";
    }
    
    // Run one generation (returns true if more generations to go)
    bool runGeneration() {
        if (currentGeneration >= config.maxGenerations) {
            state = TournamentState::Completed;
            statusMessage = "Tournament completed";
            estimatedTimeRemaining = 0.0f;
            return false;
        }
        
        // Record start time of generation
        lastGenerationStartTime = etaClock.getElapsedTime().asSeconds();
        
        // Progress output removed
        
        // Save previous winner's historical stats before resetting
        int prevWinnerHistoricalWins = 0;
        int prevWinnerHistoricalLosses = 0;
        int prevWinnerHistoricalTotalMatches = 0;
        float prevWinnerHistoricalWinRate = 0.0f;
        float prevWinnerHistoricalFitness = 0.0f;
        bool prevWinnerFound = false;
        
        if (hasPreviousWinner) {
            auto& individuals = population->getIndividuals();
            for (auto& ind : individuals) {
                if (ind.id == previousTournamentWinner.id) {
                    // Save historical stats
                    prevWinnerHistoricalWins = ind.wins;
                    prevWinnerHistoricalLosses = ind.losses;
                    prevWinnerHistoricalTotalMatches = ind.totalMatches;
                    prevWinnerHistoricalWinRate = ind.winRate;
                    prevWinnerHistoricalFitness = ind.fitness;
                    prevWinnerFound = true;
                    break;
                }
            }
        }
        
        // Reset match statistics for this generation
        for (auto& individual : population->getIndividuals()) {
            individual.resetStats();
        }
        
        // Run tournament matches
        runRoundRobinTournament();
        
        // Restore and accumulate previous winner's historical stats
        if (hasPreviousWinner && prevWinnerFound) {
            auto& individuals = population->getIndividuals();
            for (auto& ind : individuals) {
                if (ind.id == previousTournamentWinner.id) {
                    // Accumulate stats: add current tournament stats to historical
                    ind.wins += prevWinnerHistoricalWins;
                    ind.losses += prevWinnerHistoricalLosses;
                    ind.totalMatches += prevWinnerHistoricalTotalMatches;
                    // Recalculate win rate with accumulated stats
                    ind.winRate = ind.totalMatches > 0 ? 
                        (float)ind.wins / (float)ind.totalMatches : 0.0f;
                    break;
                }
            }
        }
        
        // Calculate fitness
        population->calculateAllFitness();
        population->sortByFitness();
        
        // Integrate all-time champion into population if it exists and isn't already present
        // Do this after calculating fitness so we can replace worst individual if needed
        if (hasAllTimeBest) {
            auto& individuals = population->getIndividuals();
            bool championInPopulation = false;
            
            // Check if champion is already in population
            for (const auto& ind : individuals) {
                if (ind.id == allTimeBestIndividual.id) {
                    championInPopulation = true;
                    break;
                }
            }
            
            // If champion not in population, replace worst individual with champion
            if (!championInPopulation && !individuals.empty()) {
                // Create a copy of the champion
                AIIndividual championCopy = allTimeBestIndividual;
                championCopy.generation = currentGeneration;
                // Reset stats so it competes fresh in next generation
                championCopy.resetStats();
                // Replace worst individual (last in sorted list)
                individuals.back() = championCopy;
                // Re-sort to maintain order
                population->sortByFitness();
            }
        }
        
        // Record generation stats
        GenerationStats stats;
        stats.generation = currentGeneration + 1;
        stats.avgFitness = population->getAverageFitness();
        stats.bestFitness = population->getBestFitness();
        stats.bestIndividualId = population->getBest().id;
        
        // Calculate average win rate
        float totalWinRate = 0.0f;
        for (const auto& ind : population->getIndividuals()) {
            totalWinRate += ind.winRate;
        }
        stats.avgWinRate = totalWinRate / population->getSize();
        
        // Find worst fitness
        stats.worstFitness = population->getIndividuals().back().fitness;
        
        statsHistory.push_back(stats);
        
        // Check and update all-time best
        if (stats.bestFitness > allTimeBestFitness) {
            allTimeBestFitness = stats.bestFitness;
            allTimeBestIndividual = population->getBest();  // Hace copia
            hasAllTimeBest = true;
            // Progress output removed
        }
        
        // Progress output removed
        
        // Cache top individuals for display (before evolution resets stats)
        cachedTopIndividuals.clear();
        for (int i = 0; i < min(5, (int)population->getSize()); i++) {
            const auto& ind = population->getIndividual(i);
            cachedTopIndividuals.push_back(ind);
        }
        
        // Save top models
        saveTopModels(3);
        
        // Evolve to next generation
        population->evolveNextGeneration(config.elitePercent, config.mutationRate);
        
        // Ensure all-time champion persists in the new generation
        if (hasAllTimeBest) {
            auto& individuals = population->getIndividuals();
            bool championInNewGen = false;
            
            // Check if champion is in the new generation
            for (const auto& ind : individuals) {
                if (ind.id == allTimeBestIndividual.id) {
                    championInNewGen = true;
                    break;
                }
            }
            
            // If champion not in new generation, replace worst individual with champion
            if (!championInNewGen) {
                // Sort to find worst individual
                population->sortByFitness();
                // Replace the worst individual with the champion
                AIIndividual championCopy = allTimeBestIndividual;
                championCopy.generation = currentGeneration + 1;
                championCopy.resetStats();  // Reset stats for new generation
                individuals.back() = championCopy;
            }
        }
        
        currentGeneration++;
        progress = (float)currentGeneration / (float)config.maxGenerations;
        
        // Calculate generation time and update ETA
        float currentTime = etaClock.getElapsedTime().asSeconds();
        float generationTime = currentTime - lastGenerationStartTime;
        generationTimes.push_back(generationTime);
        
        // Keep only last 5 generations for moving average
        if (generationTimes.size() > 5) {
            generationTimes.erase(generationTimes.begin());
        }
        
        // Calculate ETA
        if (!generationTimes.empty() && currentGeneration < config.maxGenerations) {
            float avgTime = 0.0f;
            for (float t : generationTimes) {
                avgTime += t;
            }
            avgTime /= generationTimes.size();
            
            int remainingGenerations = config.maxGenerations - currentGeneration;
            estimatedTimeRemaining = avgTime * remainingGenerations;
        } else {
            estimatedTimeRemaining = 0.0f;
        }
        
        return currentGeneration < config.maxGenerations;
    }
    
    // Run full tournament to completion
    void runFullTournament() {
        start();
        
        while (state == TournamentState::Running && currentGeneration < config.maxGenerations) {
            runGeneration();
        }
        
        state = TournamentState::Completed;
        statusMessage = "Tournament completed";
        
        // Progress output removed
        
        // Save final population
        population->savePopulation("models/final_population.dat");
        
        // Save top 5 for persistence between tournaments
        saveTop5ForPersistence();
        
        // Print evolution summary
        printEvolutionSummary();
    }
    
private:
    // Run round-robin tournament (all vs all)
    void runRoundRobinTournament() {
        auto& individuals = population->getIndividuals();
        int n = individuals.size();
        totalMatches = n * (n - 1);  // Each pair plays twice (home and away)
        currentMatch = 0;
        
        // Progress output removed
        
        // Each individual plays against every other individual twice
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (i == j) continue;
                
                // Setup match
                match->setupMatch(individuals[i].player, individuals[j].player);
                
                // Run match
                MatchResult result = match->runHeadless(
                    individuals[i].id, 
                    individuals[j].id,
                    config.speedMultiplier
                );
                
                // Record results
                if (result.winnerId == individuals[i].id) {
                    individuals[i].recordMatch(true);
                    individuals[j].recordMatch(false);
                } else {
                    individuals[i].recordMatch(false);
                    individuals[j].recordMatch(true);
                }
                
                currentMatch++;
                
                // Progress output removed
            }
        }
        
        // Progress output removed
    }
    
    // Save top 5 for persistence between tournaments
    void saveTop5ForPersistence() {
        population->sortByFitness();
        
        // Create directory if it doesn't exist
        #ifdef _WIN32
            system("mkdir \"models\" 2>NUL");
        #else
            system("mkdir -p \"models\"");
        #endif
        
        // Save top 5 models
        for (int i = 0; i < min(5, (int)population->getSize()); i++) {
            const auto& ind = population->getIndividual(i);
            
            if (ind.player) {
                stringstream ss;
                ss << "models/tournament_top_" << (i+1) << ".bin";
                string modelFile = ss.str();
                
                ModelSaver::saveModel(ind.player->getAgent().getNetwork(), modelFile);
                
                // Save genetic parameters and fitness
                stringstream paramsSS;
                paramsSS << "models/tournament_top_" << (i+1) << "_params.txt";
                string paramsFile = paramsSS.str();
                
                ofstream paramsOut(paramsFile);
                if (paramsOut.is_open()) {
                    paramsOut << "Rank: " << (i+1) << endl;
                    paramsOut << "ID: " << ind.id << endl;
                    paramsOut << "Fitness: " << fixed << setprecision(4) << ind.fitness << endl;
                    paramsOut << "WinRate: " << fixed << setprecision(4) << ind.winRate << endl;
                    paramsOut << "LearningRate: " << ind.genes.learningRate << endl;
                    paramsOut << "EpsilonDecay: " << ind.genes.epsilonDecay << endl;
                    paramsOut << "HiddenLayerSize: " << ind.genes.hiddenLayerSize << endl;
                    paramsOut << "DiscountFactor: " << ind.genes.discountFactor << endl;
                    paramsOut << "BatchSize: " << ind.genes.batchSize << endl;
                    paramsOut.close();
                }
            }
        }
        
        // Save tournament winner (#1) separately for next tournament
        if (population->getSize() > 0) {
            const auto& winner = population->getIndividual(0);
            if (winner.player) {
                ModelSaver::saveModel(
                    winner.player->getAgent().getNetwork(), 
                    "models/tournament_winner.bin"
                );
                
                ofstream winnerFile("models/tournament_winner_stats.txt");
                if (winnerFile.is_open()) {
                    winnerFile << "TOURNAMENT WINNER\n";
                    winnerFile << "=================\n\n";
                    winnerFile << "ID: " << winner.id << "\n";
                    winnerFile << "Generation: " << winner.generation << "\n";
                    winnerFile << "Fitness: " << fixed << setprecision(4) << winner.fitness << "\n";
                    winnerFile << "WinRate: " << fixed << setprecision(4) << winner.winRate << "\n";
                    winnerFile << "Wins: " << winner.wins << "\n";
                    winnerFile << "Losses: " << winner.losses << "\n";
                    winnerFile << "TotalMatches: " << winner.totalMatches << "\n";
                    winnerFile << "LearningRate: " << winner.genes.learningRate << "\n";
                    winnerFile << "EpsilonDecay: " << winner.genes.epsilonDecay << "\n";
                    winnerFile << "HiddenLayerSize: " << winner.genes.hiddenLayerSize << "\n";
                    winnerFile << "DiscountFactor: " << winner.genes.discountFactor << "\n";
                    winnerFile << "BatchSize: " << winner.genes.batchSize << "\n";
                    winnerFile.close();
                }
            }
        }
        
        // Save all-time champion separately
        if (hasAllTimeBest && allTimeBestIndividual.player) {
            ModelSaver::saveModel(
                allTimeBestIndividual.player->getAgent().getNetwork(), 
                "models/all_time_champion.bin"
            );
            
            ofstream champFile("models/all_time_champion_stats.txt");
            if (champFile.is_open()) {
                champFile << "ALL-TIME CHAMPION\n";
                champFile << "================\n\n";
                champFile << "ID: " << allTimeBestIndividual.id << "\n";
                champFile << "Generation: " << allTimeBestIndividual.generation << "\n";
                champFile << "Fitness: " << fixed << setprecision(4) 
                          << allTimeBestIndividual.fitness << "\n";
                champFile << "Win Rate: " << fixed << setprecision(4) 
                          << allTimeBestIndividual.winRate << "\n";
                champFile << "Record: " << allTimeBestIndividual.wins << "-" 
                          << allTimeBestIndividual.losses << "\n";
                champFile.close();
            }
        }
        
        // Progress output removed
    }
    
    // Load top 5 from previous tournament
    bool loadTop5ForPersistence(float x, float y) {
        // Check if top 1 exists
        ifstream testFile("models/tournament_top_1.bin", ios::binary);
        if (!testFile.is_open()) {
            return false;  // No previous tournament
        }
        testFile.close();
        
        // Progress output removed
        
        auto& individuals = population->getIndividuals();
        individuals.clear();
        population->setNextIdCounter(0);
        
        int loadedCount = 0;
        for (int i = 1; i <= 5; i++) {
            stringstream ss;
            ss << "models/tournament_top_" << i << ".bin";
            string modelFile = ss.str();
            
            ifstream file(modelFile, ios::binary);
            if (!file.is_open()) {
                break;  // Stop if file doesn't exist
            }
            file.close();
            
            // Load genetic parameters
            stringstream paramsSS;
            paramsSS << "models/tournament_top_" << i << "_params.txt";
            string paramsFile = paramsSS.str();
            
            GeneticParams genes;
            string savedId = "";  // Load ID from params file
            ifstream paramsIn(paramsFile);
            if (paramsIn.is_open()) {
                string line;
                while (getline(paramsIn, line)) {
                    if (line.find("ID:") != string::npos) {
                        // Extract ID (format: "ID: XXXX")
                        size_t colonPos = line.find(":");
                        if (colonPos != string::npos) {
                            savedId = line.substr(colonPos + 1);
                            // Trim whitespace
                            savedId.erase(0, savedId.find_first_not_of(" \t"));
                            savedId.erase(savedId.find_last_not_of(" \t") + 1);
                        }
                    } else if (line.find("LearningRate:") != string::npos) {
                        sscanf(line.c_str(), "LearningRate: %f", &genes.learningRate);
                    } else if (line.find("EpsilonDecay:") != string::npos) {
                        sscanf(line.c_str(), "EpsilonDecay: %f", &genes.epsilonDecay);
                    } else if (line.find("HiddenLayerSize:") != string::npos) {
                        sscanf(line.c_str(), "HiddenLayerSize: %d", &genes.hiddenLayerSize);
                    } else if (line.find("DiscountFactor:") != string::npos) {
                        sscanf(line.c_str(), "DiscountFactor: %f", &genes.discountFactor);
                    } else if (line.find("BatchSize:") != string::npos) {
                        sscanf(line.c_str(), "BatchSize: %d", &genes.batchSize);
                    }
                }
                paramsIn.close();
            }
            
            // Use saved ID if available, otherwise generate new one
            string id = (savedId.length() == 4) ? savedId : population->generateNewId();
            AIIndividual ind(id, 0, genes, x, y);
            
            // Load neural network
            if (ind.player) {
                ModelSaver::loadModel(ind.player->getAgent().getNetwork(), modelFile);
            }
            
            individuals.push_back(ind);
            loadedCount++;
        }
        
        // Fill rest of population with random individuals
        // Use config.populationSize instead of getSize() since individuals might be empty
        while (individuals.size() < config.populationSize) {
            GeneticParams genes = population->randomGenes();
            string id = population->generateNewId();
            individuals.emplace_back(id, 0, genes, x, y);
        }
        
        // Progress output removed
        return loadedCount > 0;
    }
    
    // Load previous tournament winner
    bool loadTournamentWinner(float x, float y) {
        ifstream testFile("models/tournament_winner.bin", ios::binary);
        if (!testFile.is_open()) {
            return false;  // No previous winner
        }
        testFile.close();
        
        // Load genetic parameters and stats
        ifstream statsFile("models/tournament_winner_stats.txt");
        if (!statsFile.is_open()) {
            return false;
        }
        
        GeneticParams genes;
        string savedId = "";
        int savedGeneration = 0;
        float savedFitness = 0.0f;
        float savedWinRate = 0.0f;
        int savedWins = 0;
        int savedLosses = 0;
        int savedTotalMatches = 0;
        
        string line;
        while (getline(statsFile, line)) {
            if (line.find("ID:") != string::npos) {
                size_t colonPos = line.find(":");
                if (colonPos != string::npos) {
                    savedId = line.substr(colonPos + 1);
                    savedId.erase(0, savedId.find_first_not_of(" \t"));
                    savedId.erase(savedId.find_last_not_of(" \t") + 1);
                }
            } else if (line.find("Generation:") != string::npos) {
                sscanf(line.c_str(), "Generation: %d", &savedGeneration);
            } else if (line.find("Fitness:") != string::npos) {
                sscanf(line.c_str(), "Fitness: %f", &savedFitness);
            } else if (line.find("WinRate:") != string::npos) {
                sscanf(line.c_str(), "WinRate: %f", &savedWinRate);
            } else if (line.find("Wins:") != string::npos) {
                sscanf(line.c_str(), "Wins: %d", &savedWins);
            } else if (line.find("Losses:") != string::npos) {
                sscanf(line.c_str(), "Losses: %d", &savedLosses);
            } else if (line.find("TotalMatches:") != string::npos) {
                sscanf(line.c_str(), "TotalMatches: %d", &savedTotalMatches);
            } else if (line.find("LearningRate:") != string::npos) {
                sscanf(line.c_str(), "LearningRate: %f", &genes.learningRate);
            } else if (line.find("EpsilonDecay:") != string::npos) {
                sscanf(line.c_str(), "EpsilonDecay: %f", &genes.epsilonDecay);
            } else if (line.find("HiddenLayerSize:") != string::npos) {
                sscanf(line.c_str(), "HiddenLayerSize: %d", &genes.hiddenLayerSize);
            } else if (line.find("DiscountFactor:") != string::npos) {
                sscanf(line.c_str(), "DiscountFactor: %f", &genes.discountFactor);
            } else if (line.find("BatchSize:") != string::npos) {
                sscanf(line.c_str(), "BatchSize: %d", &genes.batchSize);
            }
        }
        statsFile.close();
        
        // Create winner individual
        if (savedId.length() == 4) {
            previousTournamentWinner = AIIndividual(savedId, savedGeneration, genes, x, y);
            previousTournamentWinner.fitness = savedFitness;
            previousTournamentWinner.winRate = savedWinRate;
            previousTournamentWinner.wins = savedWins;
            previousTournamentWinner.losses = savedLosses;
            previousTournamentWinner.totalMatches = savedTotalMatches;
            
            // Load neural network
            if (previousTournamentWinner.player) {
                ModelSaver::loadModel(previousTournamentWinner.player->getAgent().getNetwork(), 
                                     "models/tournament_winner.bin");
            }
            
            hasPreviousWinner = true;
            return true;
        }
        
        return false;
    }
    
    // Save top N models
    void saveTopModels(int topN) {
        // Create models directory structure
        string genDir = "models/generation_" + to_string(currentGeneration + 1);
        
        // Create directory (cross-platform)
        #ifdef _WIN32
            system(("mkdir \"" + genDir + "\" 2>NUL").c_str());
        #else
            system(("mkdir -p \"" + genDir + "\"").c_str());
        #endif
        
        population->sortByFitness();
        
        for (int i = 0; i < min(topN, (int)population->getSize()); i++) {
            const auto& ind = population->getIndividual(i);
            
            // Create filename
            stringstream ss;
            ss << genDir << "/rank_" << (i+1) 
               << "_id_" << ind.id
               << "_fitness_" << fixed << setprecision(3) << ind.fitness 
               << ".bin";
            string modelFile = ss.str();
            
            // Save neural network
            if (ind.player) {
                ModelSaver::saveModel(ind.player->getAgent().getNetwork(), modelFile);
                
                // Save metrics
                stringstream metricsSS;
                metricsSS << genDir << "/rank_" << (i+1)
                         << "_id_" << ind.id
                         << "_metrics.txt";
                string metricsFile = metricsSS.str();
                
                ofstream metricsOut(metricsFile);
                if (metricsOut.is_open()) {
                    metricsOut << "Individual ID: " << ind.id << endl;
                    metricsOut << "Generation: " << (currentGeneration + 1) << endl;
                    metricsOut << "Rank: " << (i + 1) << endl;
                    metricsOut << "Fitness: " << fixed << setprecision(4) << ind.fitness << endl;
                    metricsOut << "Win Rate: " << fixed << setprecision(4) << ind.winRate << endl;
                    metricsOut << "Wins: " << ind.wins << endl;
                    metricsOut << "Losses: " << ind.losses << endl;
                    metricsOut << "Total Matches: " << ind.totalMatches << endl;
                    metricsOut << "\nGenetic Parameters:" << endl;
                    metricsOut << "  Learning Rate: " << ind.genes.learningRate << endl;
                    metricsOut << "  Epsilon Decay: " << ind.genes.epsilonDecay << endl;
                    metricsOut << "  Hidden Layer Size: " << ind.genes.hiddenLayerSize << endl;
                    metricsOut << "  Discount Factor: " << ind.genes.discountFactor << endl;
                    metricsOut << "  Batch Size: " << ind.genes.batchSize << endl;
                    metricsOut.close();
                }
            }
        }
        
        // Progress output removed
    }
    
    // Print evolution summary
    void printEvolutionSummary() {
        if (statsHistory.empty()) return;
        
        // Progress output removed
        
        // Save summary to file
        ofstream summaryFile("models/tournament_summary.txt");
        if (summaryFile.is_open()) {
            summaryFile << "Tournament Summary\n";
            summaryFile << "==================\n\n";
            summaryFile << "Configuration:\n";
            summaryFile << "  Population Size: " << config.populationSize << "\n";
            summaryFile << "  Generations: " << statsHistory.size() << "\n";
            summaryFile << "  Elite Percent: " << (config.elitePercent * 100) << "%\n";
            summaryFile << "  Mutation Rate: " << (config.mutationRate * 100) << "%\n\n";
            
            summaryFile << "Results:\n";
            float initialBest = statsHistory.front().bestFitness;
            float finalBest = statsHistory.back().bestFitness;
            float improvement = initialBest > 0.0f ? ((finalBest - initialBest) / initialBest) * 100.0f : 0.0f;
            summaryFile << "  Initial Best Fitness: " << fixed << setprecision(4) << initialBest << "\n";
            summaryFile << "  Final Best Fitness: " << fixed << setprecision(4) << finalBest << "\n";
            summaryFile << "  Improvement: " << fixed << setprecision(1) << improvement << "%\n\n";
            
            summaryFile << "Generation History:\n";
            for (const auto& stats : statsHistory) {
                summaryFile << "  Gen " << stats.generation << ": "
                           << "Best=" << fixed << setprecision(3) << stats.bestFitness
                           << " Avg=" << fixed << setprecision(3) << stats.avgFitness << "\n";
            }
            
            summaryFile.close();
            // Progress output removed
        }
    }
    
public:
    // Getters
    TournamentState getState() const { return state; }
    int getCurrentGeneration() const { return currentGeneration; }
    int getMaxGenerations() const { return config.maxGenerations; }
    float getProgress() const { return progress; }
    string getStatusMessage() const { return statusMessage; }
    
    AIPopulation* getPopulation() { return population; }
    const AIPopulation* getPopulation() const { return population; }
    
    // Get fitness values from last completed generation
    float getLastAverageFitness() const {
        // If we have stats history, return the last completed generation's avg fitness
        // Otherwise, calculate current average fitness from population
        if (!statsHistory.empty()) {
            return statsHistory.back().avgFitness;
        }
        // If no history yet, calculate from current population (even if fitness not yet calculated)
        if (population && population->getSize() > 0) {
            // Calculate current average fitness from population
            float sum = 0.0f;
            for (const auto& ind : population->getIndividuals()) {
                sum += ind.fitness;
            }
            return sum / population->getSize();
        }
        return 0.0f;
    }
    
    // Get current average fitness (from population, always up-to-date)
    // This ensures we always show the most current value, even during generation execution
    float getCurrentAverageFitness() const {
        if (population && population->getSize() > 0) {
            // Always calculate directly from current population state
            float sum = 0.0f;
            for (const auto& ind : population->getIndividuals()) {
                sum += ind.fitness;
            }
            return sum / population->getSize();
        }
        return 0.0f;
    }
    
    float getLastBestFitness() const {
        if (!statsHistory.empty()) {
            return statsHistory.back().bestFitness;
        }
        return population ? population->getBestFitness() : 0.0f;
    }
    
    // All-time best getters
    float getAllTimeBestFitness() const { return allTimeBestFitness; }
    const AIIndividual* getAllTimeBestIndividual() const { 
        return hasAllTimeBest ? &allTimeBestIndividual : nullptr; 
    }
    bool hasAllTimeRecord() const { return hasAllTimeBest; }
    
    const vector<GenerationStats>& getStatsHistory() const { return statsHistory; }
    
    TournamentConfig& getConfig() { return config; }
    const TournamentConfig& getConfig() const { return config; }
    
    // ETA getters
    float getEstimatedTimeRemaining() const { return estimatedTimeRemaining; }
    
    string getFormattedETA() const {
        if (currentGeneration >= config.maxGenerations) {
            return "Completed";
        }
        
        if (currentGeneration == 0 || generationTimes.empty()) {
            return "Calculating...";
        }
        
        int totalSeconds = (int)estimatedTimeRemaining;
        int hours = totalSeconds / 3600;
        int minutes = (totalSeconds % 3600) / 60;
        int seconds = totalSeconds % 60;
        
        char buffer[32];
        if (hours > 0) {
            snprintf(buffer, sizeof(buffer), "%dh %dm %ds", hours, minutes, seconds);
        } else if (minutes > 0) {
            snprintf(buffer, sizeof(buffer), "%dm %ds", minutes, seconds);
        } else {
            snprintf(buffer, sizeof(buffer), "%ds", seconds);
        }
        return string(buffer);
    }
    
    // Get top N individuals for display
    vector<AIIndividual*> getTopIndividuals(int n) {
        vector<AIIndividual*> candidates;
        
        // Add cached individuals from last completed generation
        if (!cachedTopIndividuals.empty()) {
            for (auto& ind : cachedTopIndividuals) {
                candidates.push_back(&ind);
            }
        } else {
            // Fallback to current population if cache is empty
            population->sortByFitness();
            for (int i = 0; i < min(n, (int)population->getSize()); i++) {
                candidates.push_back(&population->getIndividual(i));
            }
        }
        
        // Add all-time champion if it exists and isn't already in the list
        if (hasAllTimeBest) {
            bool championAlreadyIncluded = false;
            for (auto* candidate : candidates) {
                if (candidate->id == allTimeBestIndividual.id) {
                    championAlreadyIncluded = true;
                    break;
                }
            }
            if (!championAlreadyIncluded) {
                candidates.push_back(&allTimeBestIndividual);
            }
        }
        
        // Sort by fitness (descending) and return top N
        sort(candidates.begin(), candidates.end(),
             [](const AIIndividual* a, const AIIndividual* b) {
                 return a->fitness > b->fitness;
             });
        
        vector<AIIndividual*> top;
        for (int i = 0; i < min(n, (int)candidates.size()); i++) {
            top.push_back(candidates[i]);
        }
        
        return top;
    }
    
    // Watch a match between two specific individuals
    void watchMatch(int individual1Idx, int individual2Idx) {
        if (individual1Idx < 0 || individual1Idx >= population->getSize() ||
            individual2Idx < 0 || individual2Idx >= population->getSize()) {
            cerr << "Invalid individual indices!" << endl;
            return;
        }
        
        // Create visual match
        TournamentMatch visualMatch(windowWidth, windowHeight, config.pointsPerMatch, true);
        
        auto& ind1 = population->getIndividual(individual1Idx);
        auto& ind2 = population->getIndividual(individual2Idx);
        
        // Progress output removed
        
        visualMatch.setupMatch(ind1.player, ind2.player);
        MatchResult result = visualMatch.runVisual(ind1.id, ind2.id);
        
        // Progress output removed
    }
};

