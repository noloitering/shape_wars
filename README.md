# shape_wars
Welcome to my first ever video game! Made using the Raylib engine and transpiled to the web using emscripten. Your job is destroy as many shapes as possible, the more the sides the better! The shapes will explode into little pieces, be sure to destroy those for extra points!
# How to play
first step is to download the precompiled files from the releases tab and extract them somewhere on your computer. After you do that:
- **Windows:** run the .exe file in the extracted folder
- **Linux:** enter the directory in your terminal and run: ./shape_wars
- **Web:** If you don't use Windows or Linux and you have access to a browser then you can play the game on the web using the following link: https://ericbarrett.github.io/#shapegame
# Controls
- WASD/arrow keys = Move
- Spacebar = Dash
- LMB = Shoot
- RMB = Bomb
- P = Pause
# Config
Shape Wars uses json for it's config file. The values are described in [Game.h](https://github.com/EricBarrett/shape_wars/blob/main/Game.h)
# Compiling
- 1) first you must compile raylib (don't worry it's easy!). **If raylib is already installed on your system please update the path in build/makefile** which has been included as a submodule, so if you cloned the repository using the '--recursive' option it should be cloned as well under the 'include' directory. If not you may need call 'git submodule update' and/or 'git pull' to clone raylib. Instructions for compiling can be found here: https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux
- 2) now you can navigate to the build directory and call make
- 3) (optional) you can also compile for the web. See https://github.com/raysan5/raylib/wiki/Working-for-Web-(HTML5)#testing-raylib-game for more info
