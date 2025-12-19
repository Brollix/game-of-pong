#pragma once

#include "NeuralNetwork.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <cstdio>

using namespace std;

class ModelSaver {
public:
    // Save neural network to file
    static bool saveModel(const NeuralNetwork& network, const string& filename) {
        ofstream file(filename, ios::binary);
        if (!file.is_open()) {
            cerr << "Error: No se pudo abrir el archivo para guardar: " << filename << endl;
            return false;
        }

        vector<vector<vector<float>>> weights = network.getWeights();
        vector<vector<float>> biases = network.getBiases();

        // Write number of layers
        int numLayers = weights.size();
        file.write(reinterpret_cast<const char*>(&numLayers), sizeof(int));

        // Write layer sizes (inferred from weights)
        // First layer input size
        int firstInputSize = weights[0][0].size();
        file.write(reinterpret_cast<const char*>(&firstInputSize), sizeof(int));
        
        // Then output size for each layer
        for (int layer = 0; layer < numLayers; layer++) {
            int outputSize = weights[layer].size();
            file.write(reinterpret_cast<const char*>(&outputSize), sizeof(int));
        }

        // Write weights
        for (int layer = 0; layer < numLayers; layer++) {
            for (size_t neuron = 0; neuron < weights[layer].size(); neuron++) {
                for (size_t input = 0; input < weights[layer][neuron].size(); input++) {
                    float weight = weights[layer][neuron][input];
                    file.write(reinterpret_cast<const char*>(&weight), sizeof(float));
                }
            }
        }

        // Write biases
        for (int layer = 0; layer < numLayers; layer++) {
            for (size_t neuron = 0; neuron < biases[layer].size(); neuron++) {
                float bias = biases[layer][neuron];
                file.write(reinterpret_cast<const char*>(&bias), sizeof(float));
            }
        }

        file.close();
        cout << "Modelo guardado exitosamente en: " << filename << endl;
        return true;
    }
    
    // Guardar métricas de rendimiento
    static bool saveMetrics(float fitness, int wins, int totalGames, int episodes, float epsilon = 0.0f, const string& filename = "ai_metrics.txt") {
        ofstream file(filename);
        if (!file.is_open()) {
            cerr << "Error: No se pudo abrir el archivo para guardar métricas: " << filename << endl;
            return false;
        }
        
        file << "Fitness: " << fitness << endl;
        file << "Epsilon: " << epsilon << endl;
        file << "Wins: " << wins << endl;
        file << "TotalGames: " << totalGames << endl;
        file << "Episodes: " << episodes << endl;
        
        file.close();
        return true;
    }
    
    // Cargar métricas de rendimiento
    static bool loadMetrics(float& fitness, int& wins, int& totalGames, int& episodes, float& epsilon, const string& filename = "ai_metrics.txt") {
        ifstream file(filename);
        if (!file.is_open()) {
            // Si no existe el archivo, usar valores por defecto
            fitness = 0.0f;
            epsilon = 1.0f;
            wins = 0;
            totalGames = 0;
            episodes = 0;
            return false;
        }
        
        string line;
        while (getline(file, line)) {
            if (line.find("Fitness:") != string::npos) {
                sscanf(line.c_str(), "Fitness: %f", &fitness);
            } else if (line.find("WinRate:") != string::npos) {
                // Compatibilidad con formato antiguo
                float winRate;
                sscanf(line.c_str(), "WinRate: %f", &winRate);
                fitness = winRate;  // Convertir win rate a fitness
            } else if (line.find("Epsilon:") != string::npos) {
                sscanf(line.c_str(), "Epsilon: %f", &epsilon);
            } else if (line.find("Wins:") != string::npos) {
                sscanf(line.c_str(), "Wins: %d", &wins);
            } else if (line.find("TotalGames:") != string::npos) {
                sscanf(line.c_str(), "TotalGames: %d", &totalGames);
            } else if (line.find("Episodes:") != string::npos) {
                sscanf(line.c_str(), "Episodes: %d", &episodes);
            }
        }
        
        file.close();
        return true;
    }

    // Load neural network from file
    static bool loadModel(NeuralNetwork& network, const string& filename) {
        ifstream file(filename, ios::binary);
        if (!file.is_open()) {
            cerr << "Error: No se pudo abrir el archivo para cargar: " << filename << endl;
            return false;
        }

        // Read number of layers
        int numLayers;
        file.read(reinterpret_cast<char*>(&numLayers), sizeof(int));
        if (numLayers <= 0) {
            cerr << "Error: Número de capas inválido" << endl;
            file.close();
            return false;
        }

        // Read layer sizes
        vector<int> layerSizes;
        int firstInputSize;
        file.read(reinterpret_cast<char*>(&firstInputSize), sizeof(int));
        layerSizes.push_back(firstInputSize);

        for (int i = 0; i < numLayers; i++) {
            int outputSize;
            file.read(reinterpret_cast<char*>(&outputSize), sizeof(int));
            layerSizes.push_back(outputSize);
        }

        // Reconstruct network architecture
        // Note: This assumes the network structure matches
        // For a more robust solution, we'd need to store the full architecture

        // Read weights
        vector<vector<vector<float>>> weights(numLayers);
        for (int layer = 0; layer < numLayers; layer++) {
            int outputSize = layerSizes[layer + 1];
            int inputSize = layerSizes[layer];
            weights[layer].resize(outputSize);
            
            for (int neuron = 0; neuron < outputSize; neuron++) {
                weights[layer][neuron].resize(inputSize);
                for (int input = 0; input < inputSize; input++) {
                    float weight;
                    file.read(reinterpret_cast<char*>(&weight), sizeof(float));
                    weights[layer][neuron][input] = weight;
                }
            }
        }

        // Read biases
        vector<vector<float>> biases(numLayers);
        for (int layer = 0; layer < numLayers; layer++) {
            int outputSize = layerSizes[layer + 1];
            biases[layer].resize(outputSize);
            for (int neuron = 0; neuron < outputSize; neuron++) {
                float bias;
                file.read(reinterpret_cast<char*>(&bias), sizeof(float));
                biases[layer][neuron] = bias;
            }
        }

        file.close();

        // Set weights and biases in network
        network.setWeights(weights);
        network.setBiases(biases);

        cout << "Modelo cargado exitosamente desde: " << filename << endl;
        return true;
    }

    // Save model in a simpler text format (for debugging)
    static bool saveModelText(const NeuralNetwork& network, const string& filename) {
        ofstream file(filename);
        if (!file.is_open()) {
            cerr << "Error: No se pudo abrir el archivo para guardar: " << filename << endl;
            return false;
        }

        vector<vector<vector<float>>> weights = network.getWeights();
        vector<vector<float>> biases = network.getBiases();

        file << weights.size() << endl;  // Number of layers

        for (size_t layer = 0; layer < weights.size(); layer++) {
            file << weights[layer].size() << " " << (layer == 0 ? weights[layer][0].size() : weights[layer - 1].size()) << endl;
        }

        // Write weights
        for (size_t layer = 0; layer < weights.size(); layer++) {
            for (size_t neuron = 0; neuron < weights[layer].size(); neuron++) {
                for (size_t input = 0; input < weights[layer][neuron].size(); input++) {
                    file << weights[layer][neuron][input] << " ";
                }
                file << endl;
            }
        }

        // Write biases
        for (size_t layer = 0; layer < biases.size(); layer++) {
            for (size_t neuron = 0; neuron < biases[layer].size(); neuron++) {
                file << biases[layer][neuron] << " ";
            }
            file << endl;
        }

        file.close();
        cout << "Modelo guardado en formato texto: " << filename << endl;
        return true;
    }
};
