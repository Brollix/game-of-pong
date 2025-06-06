#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

#include <cmath>
#include <string>
#include <vector>

#include <iostream>
using namespace std;

#include "Game.h"

int main(int argc, char* argv[]) {
    Game game(1280, 720);

    if (!game.init()) {
        cerr << "Error al iniciar el juego." << endl;
        return -1;
    }

    game.run();

    return 0;
}
