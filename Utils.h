#pragma once


#include <cmath>

// =======================
// Vec2f (float version)
// =======================

struct Vec2f {
    float x = 0.0f;
    float y = 0.0f;

    Vec2f() = default;
    Vec2f(float x, float y) : x(x), y(y) {
    }

// Operadores aritméticos
    Vec2f operator+(const Vec2f& other) const {
        return Vec2f(x + other.x, y + other.y);
    }

    Vec2f operator-(const Vec2f& other) const {
        return Vec2f(x - other.x, y - other.y);
    }

    Vec2f operator*(float scalar) const {
        return Vec2f(x * scalar, y * scalar);
    }

    Vec2f operator/(float scalar) const {
        return Vec2f(x / scalar, y / scalar);
    }

    // Operadores compuestos
    Vec2f& operator+=(const Vec2f& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vec2f& operator-=(const Vec2f& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    Vec2f& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    Vec2f& operator/=(float scalar) {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    // Comparación
    bool operator==(const Vec2f& other) const {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Vec2f& other) const {
        return !(*this == other);
    }

    // Magnitude (longitud del vector)
    float magnitude() const {
        return std::sqrt(x * x + y * y);
    }

    // Normalizar (vector unitario)
    Vec2f normalized() const {
        float mag = magnitude();
        return mag != 0 ? (*this) / mag : Vec2f(0, 0);
    }

    // Producto punto
    float dot(const Vec2f& other) const {
        return x * other.x + y * other.y;
    }
};

// =======================
// Overload para salida por consola
// =======================
inline std::ostream& operator<<(std::ostream& os, const Vec2f& v) {
    os << "(" << v.x << ", " << v.y << ")";
    return os;
}
