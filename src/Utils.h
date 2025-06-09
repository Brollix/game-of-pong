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

inline float dot(const Vec2f& a, const Vec2f& b) {
    return a.x * b.x + a.y * b.y;
}

inline float magnitude(const Vec2f& v) {
    return sqrtf(v.x * v.x + v.y * v.y);
}

inline Vec2f normalize(const Vec2f& v) {
    float mag = magnitude(v);
    if (mag == 0.0f) return { 0.0f, 0.0f };
    return { v.x / mag, v.y / mag };
}

inline float relu(float x) {
    return x > 0.0f ? x : 0.0f;
}

inline float sigmoid(float x) {
    return 1.0f / (1.0f + expf(-x));
}

inline void appendVec2f(std::vector<float>& out, const Vec2f& v) {
    out.push_back(v.x);
    out.push_back(v.y);
}


// Mostrar vector en consola
inline ostream& operator<<(ostream& os, const Vec2f& v) {
    os << "(" << v.x << ", " << v.y << ")";
    return os;
}
