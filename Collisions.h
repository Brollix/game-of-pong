#pragma once

enum class CollisionSide {
    None,
    Top,
    Bottom,
    Left,
    Right
};

class Collisions {
public:
    
    static bool checkBetween(const SDL_FRect& a, const SDL_FRect& b) {
        return (a.x < b.x + b.w &&
            a.x + a.w > b.x &&
            a.y < b.y + b.h &&
            a.y + a.h > b.y);
    }
    
    static CollisionSide checkSide(const SDL_FRect& a, const SDL_FRect& b) {
        if (!checkBetween(a, b)) {
            return CollisionSide::None;
        }
        
        float overlapLeft = (a.x + a.w) - b.x;
        float overlapRight = (b.x + b.w) - a.x;
        float overlapTop = (a.y + a.h) - b.y;
        float overlapBottom = (b.y + b.h) - a.y;
        
        float minOverlapX = (overlapLeft < overlapRight) ? overlapLeft : overlapRight;
        float minOverlapY = (overlapTop < overlapBottom) ? overlapTop : overlapBottom;
        
        if (minOverlapX < minOverlapY) {
            return (overlapLeft < overlapRight) ? CollisionSide::Left : CollisionSide::Right;
        } else {
            return (overlapTop < overlapBottom) ? CollisionSide::Top : CollisionSide::Bottom;
        }
    }

    // Pared vertical (rebota horizontalmente)
    static void wallV(SDL_FRect& ballRect, Vec2f& dir, int winWidth) {
        if (ballRect.x < 0) {
            ballRect.x = 0;
            dir.x = -dir.x;
        } else if (ballRect.x + ballRect.w > winWidth) {
            ballRect.x = winWidth - ballRect.w;
            dir.x = -dir.x;
        }
    }

    // Pared horizontal (devuelve true si choca, no rebota)
    static bool wallH(SDL_FRect& ballRect, int winHeight) {
        if (ballRect.y < 0) {
            return true;
        } else if (ballRect.y + ballRect.h > winHeight) {
            return true;
        }
        return false;
    }
};