#pragma once

#include "Entity.h"
#include "EntityManager.h"
#include "nlohmann/json.hpp"
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
	int duration = 90; // bullet duration (frames converted from milliseconds in config)
	Color col = RAYWHITE; // bullet colour (RGBA)
	Color o_col = BLACK; // bullet outline colour (RGBA)
	float speed = 600; // pixels/second
};

class Game
{
	// entities
	EntityManager m_entities;
	std::shared_ptr<Entity> m_player;
	// components
	int m_score = 0;
	bool m_paused = false;
	int m_currentFrame = 0;
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
public:
	Game(const char* config);
	void run();
	void cleanup();
};
