#pragma once

#include "EntityManager.h"
#include "include/NoGUI/src/GUI.h"
#include "include/json/json.hpp"
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <iostream>
#if defined(PLATFORM_WEB)
	#include <emscripten/emscripten.h>
#endif

// default configuration
struct WindowConfig 
{
	int width = 1280; // pixels 
	int height = 720; // pixels
	int fps = 60; // frames per second
	bool full = false;  // fullscreen
	Color col = (Color) {15, 20, 10, 255}; // Background Colour (RGBA)
};

struct FontConfig 
{
	std::string file; // TTF, XNA fonts, AngelCode fonts
	Font style; // loaded from file
	float size = 20; // font size
	Color col = WHITE; // font colour (RGBA)
};

struct PlayerConfig 
{
	int sides = 3; // # of player sides
	int radius = 25; // player size (pixels)
	int c_radius = 23; // player collision size (pixels)
	int o_thick = 4; // player outline thickness (pixels)
	Color fill = DARKBLUE; // player colour (RGBA)
	Color o_col = RAYWHITE; // player outline colour (RGBA)
	float speed = 240; // pixels/second
};

struct EnemyConfig 
{
	int radius = 30; // polygon size (pixels)
	int c_radius = 28; // polygon collision size (pixels)
	int o_thick = 2; // polygon outline thickness (pixels)
	int d_life = 240; // debris lifetime (frames converted from milliseconds in config)
	Color o_col = MAROON; // polygon outline colour (RGBA)
	float spawn = 1.0; // difficulty setting (Must be greater than 0. 1.0 is easy, 2.0 spawns enemies twice as fast, etc.)
	float speed = 300; // pixels/second
};

struct BulletConfig 
{
	int radius = 5; // bullet size (pixels)
	int c_radius = 5; // bullet collision size (pixels
	int o_thick = 1; // bullet outline thickness (pixels)
	int duration = 72; // bullet duration (frames converted from milliseconds in config)
	Color col = RAYWHITE; // bullet colour (RGBA)
	Color o_col = BLACK; // bullet outline colour (RGBA)
	float speed = 1100; // pixels/second
};

// custom configuration
const std::map< std::string, Vector2 > map169 = {
	{"3840 x 2160 UHD", (Vector2){3840, 2160}},
	{"2560 x 1440 HD", (Vector2){2560, 1440}},
	{"1920 x 1080 HD", (Vector2){1920, 1080}},
	{"1280 x 720 HD", (Vector2){1280, 720}},
	{" 854 x 480 SD", (Vector2){854, 480}},
	{" 640 x 360 SD", (Vector2){640, 360}},
	{" 426 x 240 LD", (Vector2){426, 240}},
};

const std::map< std::string, int > mapFPS = {
	{"144fps UHD Gaming", 144},
	{"120fps UHD Gaming", 120},
	{" 90fps HD Gaming", 90},
	{" 72fps UHD Cinema", 72},
	{" 60fps HD Gaming", 60},
	{" 48fps HD Cinema", 48},
	{" 30fps SD Gaming", 30},
	{" 24fps SD Cinema", 24}
};

class Game
{
	// entities
	EntityManager m_entities;
	NoGUI::GUIManager m_overlay;
	std::shared_ptr< Texture2D > m_logo;
	std::shared_ptr< Entity > m_player; 
	// components
	int m_score = 0;
	int m_highScore = 0;
	bool m_paused = true;
	int m_currentFrame = 0;
	std::vector<const char*> m_labels{"NEW HIGHSCORE!", "TRY AGAIN!!", "MY GRANDMA COULD DO BETTER", "YOU CAN DO IT!", "GIT GUD LOL", "NICE TRY!", "SO CLOSE!", "YOU GOT THIS"};
	// configuration
	WindowConfig m_windowConfig;
	FontConfig m_fontConfig;
	PlayerConfig m_playerConfig;
	EnemyConfig m_enemyConfig;
	BulletConfig m_bulletConfig;
	// systems
	void sDuration();
	void sMove();
	void sInput();
	void sRender();
	void sEnemySpawner();
	void spawnPlayer();
	void spawnEnemy();
	void spawnDebris(std::shared_ptr<Entity> enemy);
	void spawnBullet(const Vector2 mousePos);
	void spawnSpecial();
	void sCollision();
	void load_menu();
	void load_settings();
public:
	Game(const char* config);
	void run();
	void cleanup();
	void setPause(bool p=true);
	bool togglePause();
};
