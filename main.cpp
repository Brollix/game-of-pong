#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

#include <cmath>
#include <string>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <filesystem>

using namespace std;

int width = 1280;
int height = 720;

#include "src/Text.h"
#include "src/FileIO.h"
#include "src/Utils.h"
#include "src/Collisions.h"
#include "src/Menu.h"
#include "src/HUD.h"
#include "src/Player.h"
#include "src/Ball.h"
#include "src/NeuralNet.h"

#include "src/Game.h"

int main(int argc, char* argv[]) {

    srand(time(NULL));

    Game game(width, height);

    if (!game.init()) {
        cerr << "Error al iniciar el juego." << endl;
        system("pause");
        return -1;
    }

    game.run();

    return 0;
}
