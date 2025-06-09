#pragma once

#include <fstream>
#include <string>
#include <vector>

using namespace std;

struct FileIO {

    template <typename T>
    static bool save(const string& filename, const T& value) {
        ofstream file(filename);
        if (!file.is_open()) return false;
        file << value;
        return true;
    }

    template <typename T>
    static bool load(const string& filename, T& value) {
        ifstream file(filename);
        if (!file.is_open()) return false;
        file >> value;
        return true;
    }

    template <typename T>
    static bool saveVector(const string& filename, const vector<T>& vec) {
        ofstream file(filename);
        if (!file.is_open()) return false;
        for (const T& v : vec)
            file << v << '\n';
        return true;
    }

    template <typename T>
    static bool loadVector(const string& filename, vector<T>& vec) {
        ifstream file(filename);
        if (!file.is_open()) return false;
        vec.clear();
        T value;
        while (file >> value)
            vec.push_back(value);
        return true;
    }
};
