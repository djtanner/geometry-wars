#include <SFML/Graphics.hpp>
#include "Game.h"
#include <iostream>

int main()
{

    // main game loop
    Game g("config.txt");

    g.run();
}