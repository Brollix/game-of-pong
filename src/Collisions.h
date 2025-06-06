#pragma once

static struct Collisions {    
    
    static bool check(const SDL_FRect a, const SDL_FRect b) {
        return SDL_HasIntersectionF(&a, &b);
    }
};