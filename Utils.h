#pragma once

struct Vec2f {
    float x = 0.0f;
    float y = 0.0f;

    Vec2f() = default;
    Vec2f(float x, float y) : x(x), y(y) {
    }

    // Operadores simples

    // Suma
    Vec2f operator+(const Vec2f& other) const {
        return Vec2f(x + other.x, y + other.y);
    }

    // Resta
    Vec2f operator-(const Vec2f& other) const {
        return Vec2f(x - other.x, y - other.y);
    }
    // Mult
    Vec2f operator*(float scalar) const {
        return Vec2f(x * scalar, y * scalar);
    }

    // Div
    Vec2f operator/(float scalar) const {
        return Vec2f(x / scalar, y / scalar);
    }

    // Operadores compuestos

    // Suma
    Vec2f& operator+=(const Vec2f& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    // Resta
    Vec2f& operator-=(const Vec2f& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    //Mult
    Vec2f& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    //Div
    Vec2f& operator/=(float scalar) {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    // Comparación

    //Igualdad
    bool operator==(const Vec2f& other) const {
        return x == other.x && y == other.y;
    }

    //Desigualdad
    bool operator!=(const Vec2f& other) const {
        return !(*this == other);
    }

    // Magnitud (longitud del vector)
    float magnitude() const {
        return sqrt(x * x + y * y);
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

// Mostrar vector en consola
inline ostream& operator<<(ostream& os, const Vec2f& v) {
    os << "(" << v.x << ", " << v.y << ")";
    return os;
}
