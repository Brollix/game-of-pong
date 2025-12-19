#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <string>
#include <vector>

using namespace std;

int width = 1280;
int height = 720;

#include "src/Game.h"

int main() {

    Game game(width, height);

    if (!game.init()) {
        cerr << "Error al iniciar el juego." << endl;
        return -1;
    }

    game.run();

    return 0;
}
