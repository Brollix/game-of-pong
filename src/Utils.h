#pragma once

#include <SFML/System/Vector2.hpp>
#include <cmath>
#include <iostream>

using namespace std;

// Usamos sf::Vector2f de SFML que ya tiene la mayoría de operadores
// Solo agregamos algunas utilidades extra que necesitamos

namespace VecUtils {
    // Magnitud (longitud del vector)
    inline float magnitude(const sf::Vector2f& v) {
        return sqrt(v.x * v.x + v.y * v.y);
    }

    // Normalizar (vector unitario)
    inline sf::Vector2f normalized(const sf::Vector2f& v) {
        float mag = magnitude(v);
        return mag != 0 ? sf::Vector2f(v.x / mag, v.y / mag) : sf::Vector2f(0, 0);
    }

    // Producto punto
    inline float dot(const sf::Vector2f& a, const sf::Vector2f& b) {
        return a.x * b.x + a.y * b.y;
    }
}

// Mostrar vector en consola
inline ostream& operator<<(ostream& os, const sf::Vector2f& v) {
    os << "(" << v.x << ", " << v.y << ")";
    return os;
}
