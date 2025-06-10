#pragma once

struct NeuralNet {
    int inputSize;
    int hiddenSize;

    int gensNoImprovement = 0;

    vector<vector<float>> hiddenWeights;
    vector<float> hiddenBias;
    vector<float> outputWeights;
    float outputBias = 0.0f;

    float fitness = 0.0f;
    float bestFitness = -INFINITY;

    float lastY = 0.0f;
    NeuralNet* bestNet = nullptr;

    NeuralNet(int inputSize = 8, int hiddenSize = 16, bool createBest = true)
        : inputSize(inputSize), hiddenSize(hiddenSize) {
        hiddenWeights.resize(hiddenSize, vector<float>(inputSize));
        hiddenBias.resize(hiddenSize);
        outputWeights.resize(hiddenSize);
        randomizeWeights();

        if (createBest){
            bestNet = new NeuralNet(inputSize, hiddenSize, false);
        }
    }

    ~NeuralNet() {
        delete bestNet;
    }

    float forward(const vector<float>& input) {
        vector<float> hiddenOutput(hiddenSize);
        for (int i = 0; i < hiddenSize; ++i) {
            float sum = hiddenBias[i];
            for (int j = 0; j < inputSize; ++j) {
                sum += hiddenWeights[i][j] * input[j];
            }
            hiddenOutput[i] = tanh(sum);
        }

        float output = outputBias;
        for (int i = 0; i < hiddenSize; ++i) {
            output += hiddenOutput[i] * outputWeights[i];
        }
        return output;
    }

    void update(Player& paddle, const Ball& ball, float dt, float playerY) {
        vector<float> input;
        appendVec2f(input, ball.pos);
        appendVec2f(input, ball.vel);
        appendVec2f(input, normalize(ball.vel));
        appendVec2f(input, ball.pos - paddle.pos);
        //input.push_back(playerY / height);

        float output = forward(input);

        if (output > 0.1f) paddle.moveUp(dt);
        else if (output < -0.1f) paddle.moveDown(dt);

        paddle.clamp(height);

        float movement = abs(paddle.pos.y - lastY);
        if (movement < 1.0f)
            fitness -= dt * 0.5f;

        lastY = paddle.pos.y;

        float distY = abs((ball.pos.y + ball.size.y * 0.5f) - (paddle.pos.y + paddle.size.y * 0.5f));
        fitness += (1.0f - (distY / height)) * dt;
        fitness += dt;
    }

    void penalizeForGoal() {
        fitness -= 10.0f;
    }

    void evaluateAndEvolve() {
        if (fitness > bestFitness) {
            bestFitness = fitness;
            gensNoImprovement = 0;
            bestNet->copyFrom(*this);
            bestNet->saveToFile("ai_data");
        } else {
            gensNoImprovement++;
            copyFrom(*bestNet);
            mutate();
        }

    }

    void randomizeWeights(float min = -1.0f, float max = 1.0f) {
        srand(static_cast<unsigned int>(time(nullptr)));
        for (auto& row : hiddenWeights){ 
                for (auto& w : row){
                    w = randFloat(min, max);
            } 
        }

        for (auto& b : hiddenBias)
            b = randFloat(min, max);

        for (auto& w : outputWeights)
            w = randFloat(min, max);

        outputBias = randFloat(min, max);
    }

    void mutate(float rate = 0.1f) {
        for (auto& row : hiddenWeights)
            for (auto& w : row)
                w += randFloat(-rate, rate);

        for (auto& b : hiddenBias)
            b += randFloat(-rate, rate);

        for (auto& w : outputWeights)
            w += randFloat(-rate, rate);

        outputBias += randFloat(-rate, rate);
    }

    void copyFrom(const NeuralNet& other) {
        hiddenWeights = other.hiddenWeights;
        hiddenBias = other.hiddenBias;
        outputWeights = other.outputWeights;
        outputBias = other.outputBias;
    }

    float relu(float x) const {
        return x > 0.0f ? x : 0.0f;
    }

    bool loadFromFile(const string& filename) {
        cout << fixed << setprecision(3);
        cout << "[AI] Loaded weights from file: " << filename << endl;

        vector<float> flatWeights;
        if (!FileIO::loadVector(filename + ".hiddenWeights", flatWeights)) return false;
        if (!FileIO::loadVector(filename + ".hiddenBias", hiddenBias)) return false;
        if (!FileIO::loadVector(filename + ".outputWeights", outputWeights)) return false;
        if (!FileIO::load(filename + ".outputBias", outputBias)) return false;

        int index = 0;
        for (auto& row : hiddenWeights)
            for (float& w : row)
                w = flatWeights[index++];

        cout << "HiddenWeights:\n";
        for (const auto& row : hiddenWeights) {
            for (float w : row) cout << setw(7) << w << " ";
            cout << endl;
        }

        cout << "HiddenBias:\n";
        for (float b : hiddenBias) cout << setw(7) << b << " ";
        cout << endl;

        cout << "OutputWeights:\n";
        for (float w : outputWeights) cout << setw(7) << w << " ";
        cout << endl;

        cout << "OutputBias: " << setw(7) << outputBias << endl;

        return true;
    }

    bool saveToFile(const string& filename) const {
        cout << fixed << setprecision(3);
        cout << "[AI] Saved weights to file: " << filename << endl;

        cout << "HiddenWeights:\n";
        for (const auto& row : hiddenWeights) {
            for (float w : row) cout << setw(7) << w << " ";
            cout << endl;
        }

        cout << "HiddenBias:\n";
        for (float b : hiddenBias) cout << setw(7) << b << " ";
        cout << endl;

        cout << "OutputWeights:\n";
        for (float w : outputWeights) cout << setw(7) << w << " ";
        cout << endl;

        cout << "OutputBias: " << setw(7) << outputBias << endl;

        return FileIO::saveVector(filename + ".hiddenWeights", flatten2D(hiddenWeights)) &&
            FileIO::saveVector(filename + ".hiddenBias", hiddenBias) &&
            FileIO::saveVector(filename + ".outputWeights", outputWeights) &&
            FileIO::save(filename + ".outputBias", outputBias);
    }


    vector<float> flatten2D(const vector<vector<float>>& matrix) const {
        vector<float> flat;
        for (const auto& row : matrix)
            flat.insert(flat.end(), row.begin(), row.end());
        return flat;
    }

    void loadAndApplyBest() {
        if (bestNet && bestNet->loadFromFile("ai_data")) {
            copyFrom(*bestNet);
        }
    }
};
