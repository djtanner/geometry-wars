#include <SFML/Graphics.hpp>
#include "Game.h"
#include <iostream>

int main()
{

    // testing vec2
    Vec2 v1(100, 100);
    Vec2 v2(200, 200);
    Vec2 v3 = v1 + v2;

    std::cout << v3.x << " " << v3.y << std::endl;

    // main game loop
    Game g("config.txt");

    g.run();
}