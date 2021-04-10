# shape_wars
Welcome to my first ever video game! Made using the Raylib engine and transpiled to the web using emscripten. Your job is destroy as many shapes as possible, the more the sides the better! The shapes will explode into little pieces, be sure to destroy those for extra points!
# How to play
first step is to download the precompiled files from the releases tab and extract them somewhere on your computer. After you do that:
- **Windows:** run the .exe file in the extracted folder
- **Linux:** enter the directory in your terminal and run: ./shape_wars
- **Web:** If you don't use Windows or Linux and you have access to a browser then you can run the game on the web. In order to do so you must host the .html, .js, and .wasm files on a web server. If you don't have a server to run it on you can run it locally with python. Please see https://github.com/raysan5/raylib/wiki/Working-for-Web-(HTML5)#testing-raylib-game for details on how to do that.
# Controls
- WASD/arrow keys = Move
- Spacebar = Dash
- LMB = Shoot
- RMB = Bomb
- P = Pause
# Config
Shape Wars uses json for it's config file. The values are described in [Game.h](https://github.com/EricBarrett/shape_wars/blob/main/Game.h)
# Compiling
I plan to add a makefile in the future. Currently there are 2 dependencies: 
1. https://github.com/raysan5/raylib
2. https://github.com/nlohmann/json 

Raylib is included within [Component.h](https://github.com/EricBarrett/shape_wars/blob/main/Component.h). You will probably need to change this path. The json parser is included in [Game.h](https://github.com/EricBarrett/shape_wars/blob/main/Game.h). It is set to a relative path of release/include/nlohmann/json. You will probably need to change this path. Once that's sorted you can finally compile using gcc. The command I use is:
```
g++ -o shape_wars main.cpp Game.cpp EntityManager.cpp Entity.cpp -Wall -std=c++17 -D_DEFAULT_SOURCE -Wno-missing-braces -s -O1 -D_DEFAULT_SOURCE -I/usr/local/include -isystem. -isystem/$PATH_TO_SRC_DIR/src -isystem/$PATH_TO_SRC_DIR/src/external -L. -L/usr/local/lib -L/$PATH_TO_SRC_DIR/src -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -DPLATFORM_DESKTOP
```
# Demo
You can demo the game at: https://ericbarrett.github.io/#shapegame
