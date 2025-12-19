#pragma once

#include <vector>
#include <cmath>
#include <random>
#include <algorithm>

using namespace std;

class NeuralNetwork {
private:
    vector<int> layerSizes;
    vector<vector<vector<float>>> weights;  // weights[layer][neuron][input]
    vector<vector<float>> biases;           // biases[layer][neuron]
    vector<vector<float>> activations;      // cache for forward pass
    vector<vector<float>> zValues;          // cache for z (before activation)
    
    random_device rd;
    mt19937 gen;
    uniform_real_distribution<float> dist;
    
    // Activation functions
    float relu(float x) {
        return max(0.0f, x);
    }
    
    float reluDerivative(float x) {
        return x > 0 ? 1.0f : 0.0f;
    }
    
    vector<float> softmax(const vector<float>& x) {
        vector<float> expX(x.size());
        float maxVal = *max_element(x.begin(), x.end());
        float sum = 0.0f;
        
        for (size_t i = 0; i < x.size(); i++) {
            expX[i] = exp(x[i] - maxVal);  // Subtract max for numerical stability
            sum += expX[i];
        }
        
        for (size_t i = 0; i < expX.size(); i++) {
            expX[i] /= sum;
        }
        
        return expX;
    }
    
    // Xavier initialization
    float xavierInit(int inputSize, int outputSize) {
        float limit = sqrt(6.0f / (inputSize + outputSize));
        uniform_real_distribution<float> xavierDist(-limit, limit);
        return xavierDist(gen);
    }
    
public:
    NeuralNetwork(vector<int> sizes) : layerSizes(sizes), gen(rd()), dist(-1.0f, 1.0f) {
        initializeNetwork();
    }
    
    // Constructor with custom hidden layer size
    NeuralNetwork(int inputSize, int hiddenSize, int outputSize) 
        : layerSizes({inputSize, hiddenSize, outputSize}), gen(rd()), dist(-1.0f, 1.0f) {
        initializeNetwork();
    }
    
    // Initialize network layers
    void initializeNetwork() {
        // Initialize weights and biases
        weights.resize(layerSizes.size() - 1);
        biases.resize(layerSizes.size() - 1);
        activations.resize(layerSizes.size());
        zValues.resize(layerSizes.size() - 1);
        
        for (size_t layer = 0; layer < layerSizes.size() - 1; layer++) {
            int inputSize = layerSizes[layer];
            int outputSize = layerSizes[layer + 1];
            
            weights[layer].resize(outputSize);
            biases[layer].resize(outputSize);
            zValues[layer].resize(outputSize);
            
            for (int neuron = 0; neuron < outputSize; neuron++) {
                weights[layer][neuron].resize(inputSize);
                for (int input = 0; input < inputSize; input++) {
                    weights[layer][neuron][input] = xavierInit(inputSize, outputSize);
                }
                biases[layer][neuron] = 0.0f;
            }
        }
    }
    
    // Forward propagation
    vector<float> forward(const vector<float>& input) {
        if (input.size() != layerSizes[0]) {
            return vector<float>(layerSizes.back(), 0.0f);
        }
        
        activations[0] = input;
        
        // Forward through hidden layers
        for (size_t layer = 0; layer < layerSizes.size() - 2; layer++) {
            int layerSize = layerSizes[layer + 1];
            activations[layer + 1].resize(layerSize);
            zValues[layer].resize(layerSize);
            
            for (int neuron = 0; neuron < layerSize; neuron++) {
                float z = biases[layer][neuron];
                for (int input = 0; input < layerSizes[layer]; input++) {
                    z += activations[layer][input] * weights[layer][neuron][input];
                }
                zValues[layer][neuron] = z;
                activations[layer + 1][neuron] = relu(z);
            }
        }
        
        // Output layer with softmax
        int outputLayer = layerSizes.size() - 2;
        int outputSize = layerSizes.back();
        activations.back().resize(outputSize);
        zValues[outputLayer].resize(outputSize);
        
        for (int neuron = 0; neuron < outputSize; neuron++) {
            float z = biases[outputLayer][neuron];
            for (int input = 0; input < layerSizes[outputLayer]; input++) {
                z += activations[outputLayer][input] * weights[outputLayer][neuron][input];
            }
            zValues[outputLayer][neuron] = z;
        }
        
        return softmax(zValues[outputLayer]);
    }
    
    // Backward propagation (for Q-Learning, we'll use a simplified version)
    void backward(const vector<float>& target, float learningRate) {
        if (target.size() != layerSizes.back()) return;
        
        int numLayers = layerSizes.size() - 1;
        
        // Calculate output error (for Q-Learning, target is the Q-value)
        vector<float> outputError(layerSizes.back());
        for (size_t i = 0; i < outputError.size(); i++) {
            outputError[i] = target[i] - zValues[numLayers - 1][i];
        }
        
        // Backpropagate through layers
        vector<vector<float>> errors(numLayers);
        errors[numLayers - 1] = outputError;
        
        // Backpropagate from output to input
        for (int layer = numLayers - 1; layer >= 0; layer--) {
            int currentLayerSize = layerSizes[layer + 1];
            int prevLayerSize = layerSizes[layer];
            
            // Calculate errors for previous layer
            if (layer > 0) {
                errors[layer - 1].resize(prevLayerSize, 0.0f);
                for (int prevNeuron = 0; prevNeuron < prevLayerSize; prevNeuron++) {
                    for (int currNeuron = 0; currNeuron < currentLayerSize; currNeuron++) {
                        errors[layer - 1][prevNeuron] += 
                            errors[layer][currNeuron] * weights[layer][currNeuron][prevNeuron] *
                            reluDerivative(zValues[layer][currNeuron]);
                    }
                }
            }
            
            // Update weights and biases
            for (int neuron = 0; neuron < currentLayerSize; neuron++) {
                float error = errors[layer][neuron];
                if (layer < numLayers - 1) {
                    error *= reluDerivative(zValues[layer][neuron]);
                }
                
                // Update bias
                biases[layer][neuron] += learningRate * error;
                
                // Update weights
                for (int input = 0; input < prevLayerSize; input++) {
                    weights[layer][neuron][input] += learningRate * error * activations[layer][input];
                }
            }
        }
    }
    
    // Get Q-values directly (before softmax) for Q-Learning
    vector<float> getQValues(const vector<float>& input) {
        if (input.size() != layerSizes[0]) {
            return vector<float>(layerSizes.back(), 0.0f);
        }
        
        activations[0] = input;
        
        // Forward through hidden layers
        for (size_t layer = 0; layer < layerSizes.size() - 2; layer++) {
            int layerSize = layerSizes[layer + 1];
            activations[layer + 1].resize(layerSize);
            
            for (int neuron = 0; neuron < layerSize; neuron++) {
                float z = biases[layer][neuron];
                for (int inputIdx = 0; inputIdx < layerSizes[layer]; inputIdx++) {
                    z += activations[layer][inputIdx] * weights[layer][neuron][inputIdx];
                }
                activations[layer + 1][neuron] = relu(z);
            }
        }
        
        // Output layer (raw Q-values, no softmax)
        int outputLayer = layerSizes.size() - 2;
        int outputSize = layerSizes.back();
        vector<float> qValues(outputSize);
        
        for (int neuron = 0; neuron < outputSize; neuron++) {
            float z = biases[outputLayer][neuron];
            for (int inputIdx = 0; inputIdx < layerSizes[outputLayer]; inputIdx++) {
                z += activations[outputLayer][inputIdx] * weights[outputLayer][neuron][inputIdx];
            }
            qValues[neuron] = z;
        }
        
        return qValues;
    }
    
    // Update Q-value for a specific action (simplified Q-Learning update)
    void updateQValue(const vector<float>& state, int action, float targetQ, float learningRate) {
        vector<float> qValues = getQValues(state);
        vector<float> target = qValues;
        target[action] = targetQ;
        backward(target, learningRate);
    }
    
    // Get weights and biases for saving/loading
    vector<vector<vector<float>>> getWeights() const { return weights; }
    vector<vector<float>> getBiases() const { return biases; }
    vector<int> getLayerSizes() const { return layerSizes; }
    
    void setWeights(const vector<vector<vector<float>>>& w) { weights = w; }
    void setBiases(const vector<vector<float>>& b) { biases = b; }
    
    // Copy weights and biases to another network
    void copyTo(NeuralNetwork& target) const {
        target.weights = this->weights;
        target.biases = this->biases;
    }
};
