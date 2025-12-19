#pragma once

#include <SFML/Graphics/Rect.hpp>

struct Collisions {    
    
    static bool check(const sf::FloatRect& a, const sf::FloatRect& b) {
        return a.intersects(b);
    }
};
