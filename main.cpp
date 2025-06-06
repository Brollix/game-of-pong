#include <SDL_main.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

#include <cmath>
#include <string>
#include <vector>

#include <iostream>
using namespace std;

int width = 1280;
int height = 720;

#include "src/Game.h"

int main(int argc, char* argv[]) {

    Game game(width, height);

    if (!game.init()) {
        cerr << "Error al iniciar el juego." << endl;
        system("pause");
        return -1;
    }

    game.run();

    return 0;
}
