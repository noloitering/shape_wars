#include "Game.h"
#include <iostream>

// hack for web
// TODO: pass in the name of the config file as argv to main function
Game g("config.json");
void main_loop()
{
  g.run();
}

int main()
{
	std::cout << "initializing game" << std::endl;
#if defined(PLATFORM_WEB)
	std::cout << "running for web" << std::endl;
    emscripten_set_main_loop(main_loop, 0, 1);
#else
	std::cout << "running for desktop" << std::endl;
    while (!WindowShouldClose())
    {
        g.run();
    }
#endif
	g.cleanup();
	CloseWindow();
	return 0;
}
