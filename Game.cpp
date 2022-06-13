#include "Game.h"

Color INVISIBLE = (Color){0, 0, 0, 1};
Color OFFGRAY = (Color){120, 120, 120, 255};

Game::Game(const char* config)
{	
	bool readConf = FileExists(config);
	if (readConf)
	{
		// read a JSON file
		std::cout << "parsing config file" << std::endl;
		std::ifstream input(config);
		nlohmann::json j_settings;
		input >> j_settings;
		//Window
		m_windowConfig.width = j_settings["Window"][0];
		m_windowConfig.height = j_settings["Window"][1];
		m_windowConfig.fps = j_settings["Window"][2];
		m_windowConfig.full = j_settings["Fullscreen"];
		m_windowConfig.col = (Color) {j_settings["Background"][0], j_settings["Background"][1], j_settings["Background"][2], 255};
		// Font
		m_fontConfig.file = j_settings["Font"][0];
		m_fontConfig.size = j_settings["Font"][1];
		m_fontConfig.col = (Color) {j_settings["Font"][2], j_settings["Font"][3], j_settings["Font"][4], 255};
		// Player
		m_playerConfig.sides = j_settings["Player"]["Sides"];
		m_playerConfig.radius = j_settings["Player"]["Size"];
		m_playerConfig.c_radius = j_settings["Player"]["Collision"];
		m_playerConfig.speed = j_settings["Player"]["Speed"];
		m_playerConfig.fill = (Color) {j_settings["Player"]["Colour"][0], j_settings["Player"]["Colour"][1], j_settings["Player"]["Colour"][2], 255};
		m_playerConfig.o_col = (Color) {j_settings["Player"]["Outline"][0], j_settings["Player"]["Outline"][1], j_settings["Player"]["Outline"][2], 255};
		m_playerConfig.o_thick = j_settings["Player"]["Outline"][3];
		// Enemy
		m_enemyConfig.radius = j_settings["Enemy"]["Size"];
		m_enemyConfig.c_radius = j_settings["Enemy"]["Collision"];
		m_enemyConfig.speed = j_settings["Enemy"]["Speed"];
		m_enemyConfig.o_col = (Color) {j_settings["Enemy"]["Outline"][0], j_settings["Enemy"]["Outline"][1], j_settings["Enemy"]["Outline"][2], 255};
		m_enemyConfig.o_thick = j_settings["Enemy"]["Outline"][3];
		m_enemyConfig.d_life = j_settings["Enemy"]["DebrisLife"];
		m_enemyConfig.d_life = m_enemyConfig.d_life * m_windowConfig.fps / 1000;
		m_enemyConfig.spawn = j_settings["Enemy"]["Spawn"];
		// Bullets
		m_bulletConfig.radius = j_settings["Bullet"]["Size"];
		m_bulletConfig.c_radius = j_settings["Bullet"]["Collision"];
		m_bulletConfig.speed = j_settings["Bullet"]["Speed"];
		unsigned char bulletRed = 255 - m_windowConfig.col.r;
		unsigned char bulletGreen = 255 - m_windowConfig.col.g;
		unsigned char bulletBlue = 255 - m_windowConfig.col.b;
		m_bulletConfig.col = (Color) {bulletRed, bulletGreen, bulletBlue, 255};
		m_bulletConfig.o_col = (Color) {j_settings["Bullet"]["Outline"][0], j_settings["Bullet"]["Outline"][1], j_settings["Bullet"]["Outline"][2], 255};
		m_bulletConfig.o_thick = j_settings["Bullet"]["Outline"][3];
		m_bulletConfig.duration = j_settings["Bullet"]["Duration"];
		m_bulletConfig.duration = m_bulletConfig.duration * m_windowConfig.fps / 1000;
	}
	else
	{
		std::cout << "Could not read config file" << std::endl;
	}
	std::cout << "initializing window" << std::endl;
	InitWindow(m_windowConfig.width, m_windowConfig.height, "Shape Wars");
    // following lines are for PLATFORM_DESKTOP only.
#if defined(PLATFORM_DESKTOP)
	std::cout << "setting FPS" << std::endl;
	SetTargetFPS(m_windowConfig.fps);
	if (m_windowConfig.full)
	{
		std::cout << "setting Fullscreen" << std::endl;
		ToggleFullscreen();
	}
#endif
	std::cout << "loading font" << std::endl;
	m_fontConfig.style = (readConf) ? LoadFont(m_fontConfig.file.c_str()) : GetFontDefault();
	std::cout << "loading GUI" << std::endl;
	m_overlay = NoGUI::GUIManager();
	load_menu();
	load_settings();
	m_overlay.getPage(1)->setActive(false);
	std::cout << "initializing Entity Manager" << std::endl;
	m_entities = EntityManager();
	std::cout << "spawning player" << std::endl;
	spawnPlayer();
	std::cout << "seeding RNG" << std::endl;
	srand( (unsigned)time(NULL) );
}

void Game::run()
{
	if ( !m_paused )
	{
		m_entities.update();
		sEnemySpawner();
		sMove();
		sCollision();
		sDuration();
		m_currentFrame++;
	}
	sInput();
	sRender();
}

void Game::sMove()
{
	m_player->cTransform->velocity.x = 0;
	m_player->cTransform->velocity.y = 0;
	// Up/Down
	if (m_player->cInput->up && !m_player->cInput->down)
	{
		m_player->cTransform->velocity.y = -1 * m_playerConfig.speed;
	}
	if (m_player->cInput->down && !m_player->cInput->up)
	{
		m_player->cTransform->velocity.y = m_playerConfig.speed;
	}
	// Left/Right
	if (m_player->cInput->left && !m_player->cInput->right)
	{
		m_player->cTransform->velocity.x = -1 * m_playerConfig.speed;
	}
	if (m_player->cInput->right && !m_player->cInput->left)
	{
		m_player->cTransform->velocity.x = m_playerConfig.speed;
	}
	// Diagonal
	if (m_player->cTransform->velocity.x != 0 && m_player->cTransform->velocity.y != 0)
	{
		m_player->cTransform->velocity.x *= cos(45);
		m_player->cTransform->velocity.y *= sin(45);
	}
}

void Game::sDuration()
{
	for (auto e : m_entities.getEntities())
	{
		if (e->cDuration)
		{
			int framesAlive = m_currentFrame - e->cDuration->frameCreated;
			unsigned char fade = 0;
			if (e->cDuration->frames < 255)
			{
				fade += 255 / e->cDuration->frames;
			}
			// if current frame is a multiple of the ceiling division of alphas max value reduce entity's alpha channel 
			else if (framesAlive % (e->cDuration->frames / 255 + (e->cDuration->frames % 255 != 0)) == 0)
			{
				fade += 1;
			}
			if (e->cShape)
			{
				e->cShape->colour.a -= fade;
				e->cShape->outlineC.a -= fade;
			}
			if (e->cLabel)
			{
				e->cLabel->colour.a -= fade;
			}
			if (framesAlive >= e->cDuration->frames)
			{
				m_entities.removeEntity(e); // is buffered so it won't invalidate our iterator
				if (e->tag() == "Bomb")
				{
					auto exp = m_entities.addEntity("Explosion");
					exp->cTransform = std::make_shared<CTransform>(e->cTransform->pos);
					exp->cShape = std::make_shared<CShape>(12, m_enemyConfig.radius * 5, (Color) {255, 75, 10, 255}, m_bulletConfig.o_col, 0);
					exp->cCollision = std::make_shared<CCollision>(m_enemyConfig.radius * 5);
					exp->cDuration = std::make_shared<CDuration>(m_windowConfig.fps / 2, m_currentFrame);
				}
			}
		}
		if (e->cDash)
		{
			if (e->cDash->active && m_currentFrame >= m_player->cDash->frameStarted + m_player->cDash->frames)
			{
				std::cout << "Dash has cooled down" << std::endl;
				e->cDash->active = false;
			}
		}
	}
}

void Game::sEnemySpawner()
{
	// increase spawn rate by .5 second every 30 secs with a max spawnrate of .1 second and a base spawnrate of 3 / config value seconds
	// TODO: add these settings in config??
	int spawnRate = 3000 - 500 * (m_currentFrame / m_windowConfig.fps / 30);
	int maxRate = 100;
	if ( int(m_currentFrame * 1000 * m_enemyConfig.spawn / m_windowConfig.fps) % ((spawnRate > maxRate) ? spawnRate : maxRate) == 0)
	{
		std::cout << "Spawning Enemy" << std::endl;
		spawnEnemy();
	}
}

void Game::sInput()
{
	if ( IsKeyPressed(KEY_P) || m_overlay.getPage()->getElement(0)->getFocus() )
	{
		m_overlay.getPage()->getElement(0)->setFocus(false);
		togglePause();
	}
	if ( m_overlay.getPage()->getElement(1)->getFocus() )
	{
		m_overlay.getPage()->getElement(1)->setFocus(false);
		setPause(true);
	}
	m_player->cInput->up = IsKeyDown(KEY_W) || IsKeyDown(KEY_UP);
	m_player->cInput->down = IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN);
	m_player->cInput->left = IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT);
	m_player->cInput->right = IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT);
	m_player->cInput->dash = IsKeyDown(KEY_SPACE);
	m_player->cInput->shoot = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
	m_player->cInput->special = IsMouseButtonDown(MOUSE_RIGHT_BUTTON);
	if ( !m_paused )
	{
		if (m_player->cInput->shoot && m_entities.getEntities("Bullet").empty())
		{
			spawnBullet(GetMousePosition());
		}
		if (m_player->cInput->special)
		{
			spawnSpecial();
		}
		if (m_player->cInput->dash && !(m_player->cDash->active) && m_currentFrame >= m_player->cDash->frameStarted + m_player->cDash->frames + m_player->cDash->delay)
		{
			std::cout << "Dashing!" << std::endl;
			m_player->cDash->active = true;
			m_player->cDash->frameStarted = m_currentFrame;
		}
	}
}

void Game::sRender()
{
	BeginDrawing();
		ClearBackground(m_windowConfig.col);
		// C style string fuckery
		char scoreText[28] = "SCORE: ";
		char scoreNum[21];
		sprintf(scoreNum, "%d", m_score);
		strcat(scoreText, scoreNum);
		Vector2 scorePos = (Vector2) {8, m_fontConfig.size + 2};
		char highScoreText[33] = "HIGH SCORE: ";
		char highScoreNum[21];
		sprintf(highScoreNum, "%d", m_highScore);
		strcat(highScoreText, highScoreNum);
		Vector2 highScorePos = (Vector2) {8, m_fontConfig.size * 2 + 2};
		char timeText[28] = "TIME: ";
		char timeNum[21];
		sprintf(timeNum, "%d", m_currentFrame / m_windowConfig.fps);
		strcat(timeText, timeNum);
		Vector2 timePos = (Vector2) {GetScreenWidth() - 8 * m_fontConfig.size,  m_fontConfig.size + 2};
		DrawTextEx(m_fontConfig.style, scoreText, scorePos, m_fontConfig.size, 2, m_fontConfig.col);
		DrawTextEx(m_fontConfig.style, highScoreText, highScorePos, m_fontConfig.size, 2, m_fontConfig.col);
		DrawTextEx(m_fontConfig.style, timeText, timePos, m_fontConfig.size, 2, m_fontConfig.col);
		for (auto e : m_entities.getEntities())
		{
			if (e->cTransform)
			{
				if (!m_paused)
				{
					float posUpdateX = e->cTransform->velocity.x * (1.0f / (float)(m_windowConfig.fps));
					float posUpdateY = e->cTransform->velocity.y * (1.0f / (float)(m_windowConfig.fps));
					if (e->cDash)
					{
						if (e->cDash->active)
						{
							posUpdateX *= e->cDash->speedMod;
							posUpdateY *= e->cDash->speedMod;
						}
					}
					e->cTransform->pos.x += posUpdateX;
					e->cTransform->pos.y += posUpdateY;
				}
				if (e->cShape)
				{
					DrawPoly(e->cTransform->pos, e->cShape->sides, e->cShape->radius, e->cTransform->rotation, e->cShape->colour);
					for (int i = 0; i < e->cShape->outlineW; i++)
					{
						DrawPolyLines(e->cTransform->pos, e->cShape->sides, e->cShape->radius + i, e->cTransform->rotation, e->cShape->outlineC);
					}
					// full rotation every 4 seconds
					e->cTransform->rotation += 90.0 / (float) m_windowConfig.fps;
				}
				if (e->cLabel)
				{
					DrawTextEx(m_fontConfig.style, e->cLabel->text, e->cTransform->pos, e->cLabel->size, 2, e->cLabel->colour);
				}
			}
		}
		if ( m_paused )
		{
			m_overlay.update();
			m_overlay.render();
			DrawTexture(*(m_logo.get()), m_windowConfig.width / 2 - m_logo->width / 2, 2, WHITE); // TODO: draw logo with NoGUI
		}
	EndDrawing();
}

void Game::spawnPlayer()
{
	Vector2 center = (Vector2) {static_cast<float>(GetScreenWidth() / 2), static_cast<float>(GetScreenHeight() / 2)};
	// reset game
	const char* labelText = "";
	auto cleanup = m_entities.getEntities();
	if (!cleanup.empty())
	{
		labelText = m_labels[GetRandomValue(1, m_labels.size() - 1)];
		for (auto e : cleanup)
		{
			m_entities.removeEntity(e);
		}
	}
	if (m_score && m_score >= m_highScore)
	{
		m_highScore = m_score;
		labelText = m_labels[0];
	}
	m_score = 0;
	m_currentFrame = 0;
	// spawn message
	auto label = m_entities.addEntity("Label");
	label->cLabel = std::make_shared<CLabel>(labelText, m_fontConfig.size * 4, m_fontConfig.col);
	Vector2 labelBounds = MeasureTextEx(m_fontConfig.style, label->cLabel->text,  m_fontConfig.size * 2, 2);
	label->cTransform = std::make_shared<CTransform>((Vector2) {(center.x - labelBounds.x), (center.y - labelBounds.y)});
	label->cDuration = std::make_shared<CDuration>(3 * m_windowConfig.fps / m_enemyConfig.spawn, m_currentFrame);
	// create the player
	m_player = m_entities.addEntity("Player");
	m_player->cCollision = std::make_shared<CCollision>(m_playerConfig.c_radius);
	m_player->cInput = std::make_shared<CInput>();
	m_player->cShape = std::make_shared<CShape>(m_playerConfig.sides, m_playerConfig.radius, m_playerConfig.fill, m_playerConfig.o_col, m_playerConfig.o_thick);
	m_player->cTransform = std::make_shared<CTransform>((Vector2) {(center.x - m_player->cShape->radius / 2), (center.y - m_player->cShape->radius / 2)});
	// TODO: settings in config??
	m_player->cDash = std::make_shared<CDash>(m_windowConfig.fps / 4, 0, m_windowConfig.fps, 2.0, false);
}

void Game::spawnEnemy()
{
	auto enemy = m_entities.addEntity("Enemy");
	float diameter = m_enemyConfig.radius * 2;
	int colourDiff = 0;
	// ensure colour of enemy differs from background or player
	Color fill = (Color) {static_cast<unsigned char>(GetRandomValue(0, 255)), static_cast<unsigned char>(GetRandomValue(0, 255)), static_cast<unsigned char>(GetRandomValue(0, 255)), 255};
	colourDiff += abs(fill.r - m_player->cShape->colour.r);
	colourDiff += abs(fill.g - m_player->cShape->colour.g);
	colourDiff += abs(fill.b - m_player->cShape->colour.b);
	if (colourDiff < 90)
	{
		// remove 90 from the highest value
		if (fill.r > fill.g && fill.r > fill.b)
		{
			if (fill.r > 90)
			{
				fill.r -= 90;
			}
			else
			{
				fill.r += 90;
			}
		}
		if (fill.g > fill.r && fill.g > fill.b)
		{
			if (fill.g > 90)
			{
				fill.g -= 90;
			}
			else
			{
				fill.g += 90;
			}
		}
		if (fill.b > fill.g && fill.b > fill.r)
		{
			if (fill.b > 90)
			{
				fill.b -= 90;
			}
			else
			{
				fill.b += 90;
			}
		}
	}
	colourDiff = 0;
	colourDiff += abs(fill.r - m_windowConfig.col.r);
	colourDiff += abs(fill.g - m_windowConfig.col.g);
	colourDiff += abs(fill.b - m_windowConfig.col.b);
	if (colourDiff < 90)
	{
		// add 90 from the lowest value
		if (fill.r < fill.g && fill.r < fill.b)
		{
			if (fill.r >= 165)
			{
				fill.r -= 90;
			}
			else
			{
				fill.r += 90;
			}
		}
		if (fill.g < fill.r && fill.g < fill.b)
		{
			if (fill.g >= 165)
			{
				fill.g -= 90;
			}
			else
			{
				fill.g += 90;
			}
		}
		if (fill.b < fill.g && fill.b < fill.r)
		{
			if (fill.b >= 165)
			{
				fill.b -= 90;
			}
			else
			{
				fill.b += 90;
			}
		}
	}
	float posX = GetRandomValue(diameter, (GetScreenWidth() - diameter));
	float posY = GetRandomValue(diameter, (GetScreenHeight() - diameter));
	Vector2 pos = (Vector2) {posX, posY};
	int playerLeft = m_player->cTransform->pos.x - m_player->cShape->radius * 2;
	int playerRight = m_player->cTransform->pos.x + m_player->cShape->radius * 2;
	int playerTop = m_player->cTransform->pos.y - m_player->cShape->radius * 2;
	int playerBot = m_player->cTransform->pos.y + m_player->cShape->radius * 2;
	Vector2 vel = (Vector2) {static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/m_enemyConfig.speed)), static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/m_enemyConfig.speed))};
	// ensure the enemy doesn't spawn too close to the player
	if ((pos.x - playerLeft) <= (playerRight - playerLeft))
	{
		if (m_player->cTransform->pos.x < GetScreenWidth() / 2)
		{
			pos.x += m_player->cShape->radius * 2;
			if (vel.x < 0)
			{
				vel.x *= -1;
			}
		}
		else
		{
			pos.x -= m_player->cShape->radius * 2;
			if (vel.x > 0)
			{
				vel.x *= -1;
			}
		}
	}
	if ((pos.y - playerTop) <= (playerBot - playerTop))
	{
		if (m_player->cTransform->pos.y < GetScreenHeight() / 2)
		{
			pos.y += m_player->cShape->radius * 2;
			if (vel.y < 0)
			{
				vel.y *= -1;
			}
		}
		else
		{
			pos.y -= m_player->cShape->radius * 2;
			if (vel.y > 0)
			{
				vel.y *= -1;
			}
		}
	}
	enemy->cTransform = std::make_shared<CTransform>(pos, vel);
	enemy->cShape = std::make_shared<CShape>(GetRandomValue(3, 8), m_enemyConfig.radius, fill, m_enemyConfig.o_col, m_enemyConfig.o_thick);
	enemy->cCollision =  std::make_shared<CCollision>(m_enemyConfig.c_radius);
	enemy->cScore = std::make_shared<CScore>(100 * enemy->cShape->sides);
}

void Game::spawnDebris(std::shared_ptr<Entity> enemy)
{
	float angle = enemy->cTransform->rotation * PI / 180.0;
	int speed = sqrt(enemy->cTransform->velocity.x * enemy->cTransform->velocity.x + enemy->cTransform->velocity.y * enemy->cTransform->velocity.y);
	for (int i = 0; i < enemy->cShape->sides; i++)
	{
		Vector2 vel = (Vector2) {speed * cos(angle), speed * sin(angle)};
		auto e = m_entities.addEntity("Debris");
		e->cShape = std::make_shared<CShape>(enemy->cShape->sides, (enemy->cShape->radius / (enemy->cShape->sides - 1)), enemy->cShape->colour, enemy->cShape->outlineC, 1 || (enemy->cShape->outlineW / (enemy->cShape->sides - 1)));
		e->cTransform = std::make_shared<CTransform>(enemy->cTransform->pos, vel, angle);
		e->cCollision = std::make_shared<CCollision>(m_enemyConfig.c_radius / enemy->cShape->sides);
		e->cScore = std::make_shared<CScore>(enemy->cScore->val * 2);
		e->cDuration = std::make_shared<CDuration>(m_enemyConfig.d_life, m_currentFrame);
		angle += (2 * PI / enemy->cShape->sides);
	}
	m_score += enemy->cScore->val;
	m_highScore = (m_score >= m_highScore) ? m_score : m_highScore;
	m_entities.removeEntity(enemy);
}

void Game::spawnBullet(const Vector2 mousePos)
{
	// TODO: try the fast inverse square root algorithm
	auto b = m_entities.addEntity("Bullet");
	Vector2 origin = m_player->cTransform->pos;
	Vector2 direction = (Vector2) {(mousePos.x - origin.x), (mousePos.y - origin.y)};
	
	float magSquare = direction.x * direction.x + direction.y * direction.y;
	float mag = sqrt(magSquare);
	direction.x = direction.x / mag * m_bulletConfig.speed;
	direction.y = direction.y / mag * m_bulletConfig.speed;
	b->cTransform = std::make_shared<CTransform>(origin, direction);
	b->cShape = std::make_shared<CShape>(10, m_bulletConfig.radius, m_bulletConfig.col, m_bulletConfig.o_col, m_bulletConfig.o_thick);
	b->cCollision = std::make_shared<CCollision>(m_enemyConfig.c_radius);
	b->cDuration = std::make_shared<CDuration>(m_bulletConfig.duration, m_currentFrame);
}

void Game::spawnSpecial()
{
	// TODO: bomb setings in config file?
	int lastCreated = -1 * m_windowConfig.fps;
	for (auto e : m_entities.getEntities("Bomb"))
	{
		if (e->cDuration->frameCreated > lastCreated)
		{
			lastCreated = e->cDuration->frameCreated;
		}
	}
	if (m_currentFrame > lastCreated + m_windowConfig.fps / 2)
	{
		auto b = m_entities.addEntity("Bomb");
		b->cTransform = std::make_shared<CTransform>(m_player->cTransform->pos);
		b->cShape = std::make_shared<CShape>(4, m_bulletConfig.radius, m_bulletConfig.col, m_bulletConfig.o_col, m_bulletConfig.o_thick);
		b->cDuration =  std::make_shared<CDuration>(m_bulletConfig.duration, m_currentFrame);
	}
}

void Game::sCollision()
{
	// Player
	// horizontal window bounds
	if (m_player->cTransform->pos.x - m_player->cCollision->radius <= 0)
	{
		m_player->cTransform->velocity.x = m_playerConfig.speed;
	}
	else if (m_player->cTransform->pos.x + m_player->cCollision->radius > GetScreenWidth())
	{
		m_player->cTransform->velocity.x = -1 * m_playerConfig.speed;
	}
	// vertical window bounds
	if (m_player->cTransform->pos.y - m_player->cCollision->radius <= 0)
	{
		m_player->cTransform->velocity.y = m_playerConfig.speed;
	}
	else if (m_player->cTransform->pos.y + m_player->cCollision->radius > GetScreenHeight())
	{
		m_player->cTransform->velocity.y = -1 * m_playerConfig.speed;
	}
	// Enemies
	for (auto enemy : m_entities.getEntities("Enemy"))
	{
		if (enemy->cTransform && enemy->cCollision)
		{
			// collision with player
			if (CheckCollisionCircles(m_player->cTransform->pos, m_player->cCollision->radius, enemy->cTransform->pos, enemy->cCollision->radius))
			{
				if (m_player->cDash->active)
				{
					spawnDebris(enemy);
				}
				else
				{
					spawnPlayer();
					return;
				}
			}
			// horizontal window bounds
			if (enemy->cTransform->pos.x - enemy->cCollision->radius <= 0 || enemy->cTransform->pos.x + enemy->cCollision->radius > GetScreenWidth())
			{
				enemy->cTransform->velocity.x *= -1;
			}
			// vertical window bounds
			if (enemy->cTransform->pos.y - enemy->cCollision->radius <= 0 || enemy->cTransform->pos.y + enemy->cCollision->radius > GetScreenHeight())
			{
				enemy->cTransform->velocity.y *= -1;
			}
		}
		// bullets
		for (auto bullet : m_entities.getEntities("Bullet"))
		{
			if (CheckCollisionCircles(enemy->cTransform->pos, enemy->cCollision->radius, bullet->cTransform->pos, bullet->cCollision->radius))
			{
				m_entities.removeEntity(bullet);
				spawnDebris(enemy);
			}
		}
		// special weapon
		for (auto exp : m_entities.getEntities("Explosion"))
		{
			if (CheckCollisionCircles(enemy->cTransform->pos, enemy->cCollision->radius, exp->cTransform->pos, exp->cCollision->radius))
			{
				m_entities.removeEntity(enemy);
				m_score += enemy->cScore->val;
				m_highScore = (m_score >= m_highScore) ? m_score : m_highScore;
			}
			if (CheckCollisionCircles(m_player->cTransform->pos, m_player->cCollision->radius, exp->cTransform->pos, exp->cCollision->radius))
			{
				spawnPlayer();
				return;
			}
		}
	}
	// Debris
	for (auto debris : m_entities.getEntities("Debris"))
	{
		if (debris->cTransform && debris->cCollision)
		{
			// collision with player
			if (CheckCollisionCircles(m_player->cTransform->pos, m_player->cCollision->radius, debris->cTransform->pos, debris->cCollision->radius))
			{
				if (m_player->cDash->active)
				{
					m_entities.removeEntity(debris);
					m_score += debris->cScore->val;
					m_highScore = (m_score >= m_highScore) ? m_score : m_highScore;
				}
				else
				{
					spawnPlayer();
					return;
				}
			}
		}
		// bullets
		for (auto bullet : m_entities.getEntities("Bullet"))
		{
			if (CheckCollisionCircles(debris->cTransform->pos, debris->cCollision->radius, bullet->cTransform->pos, bullet->cCollision->radius))
			{
				m_entities.removeEntity(bullet);
				m_entities.removeEntity(debris);
				m_score += debris->cScore->val;
				m_highScore = (m_score >= m_highScore) ? m_score : m_highScore;
			}
		}
		for (auto exp : m_entities.getEntities("Explosion"))
		{
			if (CheckCollisionCircles(debris->cTransform->pos, debris->cCollision->radius, exp->cTransform->pos, exp->cCollision->radius))
			{
				m_entities.removeEntity(debris);
				m_score += debris->cScore->val;
				m_highScore = (m_score >= m_highScore) ? m_score : m_highScore;
			}
		}
	}
}

void Game::load_menu()
{
	float posx = m_windowConfig.width / 2;
	Vector2 radius = {m_windowConfig.width / 6, m_windowConfig.height / 20};
	
	if ( m_logo )
	{
		UnloadTexture(*(m_logo.get()));
	}
	m_overlay.clear();
	
	NoGUI::Style playStyle = {OFFGRAY, BLACK, (Vector2){posx, m_windowConfig.height / 2 + radius.y + 10}, radius, 4, 5, 0};
	NoGUI::Style setStyle = {OFFGRAY, BLACK, (Vector2){posx, playStyle.pos.y + radius.y * 3}, radius, 4, 5, 0};
	if ( FileExists("../assets/logo-720p.png") )
	{
		std::cout << "found logo asset!" << std::endl;
	}
	else
	{
		std::cout << "no logo asset!" << std::endl;
	}
	m_logo = std::make_shared< Texture2D >(LoadTexture("../assets/logo-720p.png"));
//	NoGUI::Style logoStyle = {WHITE, INVISIBLE, (Vector2){posx, 20}, (Vector2){300, 400}, 4, 0, 0};
	
	std::shared_ptr< NoGUI::Page > page = std::make_shared< NoGUI::Page >();
	page->addComponents("Label", std::make_shared< NoGUI::CContainer >());
	page->getComponents("Label")->addComponent< NoGUI::CText >();
	page->getComponents("Label")->getComponent< NoGUI::CText >().align = NoGUI::TextAlign::CENTER;
//	page->addComponents("Logo", std::make_shared< NoGUI::CContainer >());
//	page->getComponents("Logo")->addComponent< NoGUI::CImage >(texture);
	
	std::shared_ptr< NoGUI::Element > playButton = page->addElement< NoGUI::Button >(playStyle, "Label", "Play");
	std::shared_ptr< NoGUI::Element > setButton = page->addElement< NoGUI::Button >(setStyle, "Label", "Settings");
//	std::shared_ptr< NoGUI::Element > logo = page->addElement< NoGUI::Element >(logoStyle, "Logo", "");
	m_overlay.addPage(page);
	m_overlay.update();
}

void Game::load_settings()
{
	float posx = m_windowConfig.width / 1.5;
	Vector2 radius = {m_windowConfig.width / 6, m_windowConfig.height / 20};
	
	NoGUI::Style resStyle = {INVISIBLE, WHITE, (Vector2){posx, m_windowConfig.height / 2}, radius, 4, 5, 0};
	
	std::shared_ptr< NoGUI::Page > spage = m_overlay.addPage();
	spage->addComponents("Label", std::make_shared< NoGUI::CContainer >());
	spage->getComponents("Label")->addComponent< NoGUI::CText >();
	spage->getComponents("Label")->getComponent< NoGUI::CText >().align = NoGUI::TextAlign::CENTER;
	
	std::shared_ptr< NoGUI::Element > resDrop = spage->addElement< NoGUI::Input >(resStyle, "Label", "1280 x 720 HD");
	std::shared_ptr< NoGUI::DropDown > resDown = m_overlay.addDropDown(resDrop, NoGUI::TextWrap::DOWN);
	resDown->addComponents("Option", std::make_shared< NoGUI::CContainer >());
	resDown->getComponents("Option")->addComponent< NoGUI::CText >();
	resDown->getComponents("Option")->getComponent< NoGUI::CText >().align = NoGUI::TextAlign::CENTER;
	for (auto i = map169.rbegin(); i != map169.rend(); ++i)
	{
		resDown->addElement< NoGUI::Button >("Option", i->first);
	}
	
	m_overlay.addPage(spage);
	m_overlay.update();
}

bool Game::togglePause()
{
	setPause(!(m_paused));
	
	return m_paused;
}

void Game::setPause(bool p)
{
	m_overlay.getPage(0)->setActive(false);
	m_paused = p;
	m_overlay.getPage(1)->setActive(m_paused);
}

void Game::cleanup()
{
	UnloadFont(m_fontConfig.style); // we use smart pointers for everything else
	UnloadTexture(*(m_logo.get()));
}
