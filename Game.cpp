#include "Game.h"

bool parse_config(GameConfig& config, const char* file)
{
	if ( FileExists(file) )
	{
		std::ifstream input(file);
		nlohmann::json j_settings;
		input >> j_settings;
		parse_window(config.window, j_settings);
		parse_font(config.font, j_settings);
		parse_player(config.player, j_settings);
		parse_enemy(config.enemy, j_settings);
		parse_bullet(config.bullet, j_settings);
		
		return true;
	}
	else
	{
		
		return false;
	}
}

void parse_window(WindowConfig& config, const nlohmann::json& json)
{
	//Window
	config.width = json["Window"][0];
	config.height = json["Window"][1];
	config.fps = json["Window"][2];
	config.full = json["Fullscreen"];
	config.col = (Color) {json["Background"][0], json["Background"][1], json["Background"][2], 255};
}

void parse_font(FontConfig& config, const nlohmann::json& json)
{
	config.file = json["Font"][0];
	config.size = json["Font"][1];
	config.col = (Color) {json["Font"][2], json["Font"][3], json["Font"][4], 255};
}

void parse_player(PlayerConfig& config, const nlohmann::json& json)
{
	config.sides = json["Player"]["Sides"];
	config.radius = json["Player"]["Size"];
	config.c_radius = json["Player"]["Collision"];
	config.speed = json["Player"]["Speed"];
	config.fill = (Color) {json["Player"]["Colour"][0], json["Player"]["Colour"][1], json["Player"]["Colour"][2], 255};
	config.o_col = (Color) {json["Player"]["Outline"][0], json["Player"]["Outline"][1], json["Player"]["Outline"][2], 255};
	config.o_thick = json["Player"]["Outline"][3];
}

void parse_enemy(EnemyConfig& config, const nlohmann::json& json)
{
	config.radius = json["Enemy"]["Size"];
	config.c_radius = json["Enemy"]["Collision"];
	config.speed = json["Enemy"]["Speed"];
	config.o_col = (Color) {json["Enemy"]["Outline"][0], json["Enemy"]["Outline"][1], json["Enemy"]["Outline"][2], 255};
	config.o_thick = json["Enemy"]["Outline"][3];
	config.d_life = json["Enemy"]["DebrisLife"];
	int fps = json["Window"][2];
	config.d_life = config.d_life * fps / 1000;
	config.spawn = json["Enemy"]["Spawn"];
}

void parse_bullet(BulletConfig& config, const nlohmann::json& json)
{
	config.radius = json["Bullet"]["Size"];
	config.c_radius = json["Bullet"]["Collision"];
	config.speed = json["Bullet"]["Speed"];
	unsigned char backRed = json["Background"][0];
	unsigned char backGreen = json["Background"][1];
	unsigned char backBlue = json["Background"][2];
	unsigned char bulletRed = 255 - backRed;
	unsigned char bulletGreen = 255 - backGreen;
	unsigned char bulletBlue = 255 - backBlue;
	config.col = (Color) {bulletRed, bulletGreen, bulletBlue, 255};
	config.o_col = (Color) {json["Bullet"]["Outline"][0], json["Bullet"]["Outline"][1], json["Bullet"]["Outline"][2], 255};
	config.o_thick = json["Bullet"]["Outline"][3];
	config.duration = json["Bullet"]["Duration"];
	int fps = json["Window"][2];
	config.duration = config.duration * fps / 1000;
}

int Background::addCol(const Color& col)
{
	colours.push_back(col);
	
	return colours.size();
}

void Background::setCol(const std::vector< Color >& col)
{
	colours = col;
}

int Background::removeCol(int index)
{
	colours.erase(colours.begin() + index);
	
	return colours.size();
}

void Background::step()
{
	if ( colours.size() > 1 )
	{
		int next;
		if ( index + 1 > colours.size() - 1 )
		{
			next = 0;
		}
		else
		{
			next = index + 1;
		}
		int framesRem = (hframes + tframes) - (frames % (tframes + hframes));
		if ( framesRem <= tframes ) // transition
		{
			Color nextCol = colours.at(next);
			int diffr = (currCol.r - nextCol.r);
			int diffg = (currCol.g - nextCol.g);
			int diffb = (currCol.b - nextCol.b);
			int redInc = (double)diffr / framesRem;
			int greenInc = (double)diffg / framesRem;
			int blueInc = (double)diffb / framesRem;
			currCol.r -= redInc;
			currCol.g -= greenInc;
			currCol.b -= blueInc;
//			std::cout << framesRem << std::endl;
//			std::cout << "rgba: {" << (int)currCol.r << ", " << (int)currCol.g << ", " << (int)currCol.b << "}" << std::endl;
//			std::cout << "dest rgba: {" << (int)nextCol.r << ", " << (int)nextCol.g << ", " << (int)nextCol.b << "}" << std::endl;
//			std::cout << "dest index: " << next << std::endl;
//			std::cout << "rgba diff: {" << diffr << ", " << diffg << ", " << diffb << "}" << std::endl;
//			std::cout << "rgba step: {" << redInc << ", " << greenInc << ", " << blueInc << "}" << std::endl;
			if ( framesRem == 1 )
			{
				index = next;
			}
		}
	}
	entities.update();
	for (auto e : entities.getEntities())
	{
		if (e->cDuration)
		{
			int framesAlive = frames - e->cDuration->frameCreated;
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
			if (framesAlive >= e->cDuration->frames)
			{
				std::cout << "despawning " << e->tag() << " " << e->id() << std::endl;
				entities.removeEntity(e); // is buffered so it won't invalidate our iterator
			}
		}
	}
	frames++;
}

void Background::draw()
{
	ClearBackground(currCol);
}

void Background::spawner()
{
	// spawn based on difficulty
	size_t spawnRate = 45 - 15 * (enemyConfig.spawn - 1);
	if ( frames % spawnRate == 0)
	{
		std::cout << "Spawning Background Enemy" << std::endl;
		spawnEntity();
	}
}

void Background::spawnEntity()
{
	auto enemy = entities.addEntity("Enemy");
	float diameter = enemyConfig.radius * 2;
	Color fill = (Color) {static_cast<unsigned char>(GetRandomValue(0, 255)), static_cast<unsigned char>(GetRandomValue(0, 255)), static_cast<unsigned char>(GetRandomValue(0, 255)), 255};
	float posX = GetRandomValue(diameter, (GetScreenWidth() - diameter));
	float posY = GetRandomValue(diameter, (GetScreenHeight() - diameter));
	Vector2 pos = (Vector2) {posX, posY};
	float randNum1 = static_cast <float> (rand());
	float randNum2 = static_cast <float> (rand());
	float divisor = static_cast <float> (RAND_MAX/ (enemyConfig.speed * 2));
	Vector2 vel = (Vector2){randNum1 / divisor - enemyConfig.speed, randNum2 / divisor - enemyConfig.speed};
	enemy->cTransform = std::make_shared<CTransform>(pos, vel);
	enemy->cShape = std::make_shared<CShape>(GetRandomValue(3, 8), enemyConfig.radius, fill, enemyConfig.o_col, enemyConfig.o_thick);
	enemy->cDuration = std::make_shared< CDuration >(enemyConfig.d_life, frames);
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
		m_player->cTransform->velocity.y = -1 * config.player.speed;
	}
	if (m_player->cInput->down && !m_player->cInput->up)
	{
		m_player->cTransform->velocity.y = config.player.speed;
	}
	// Left/Right
	if (m_player->cInput->left && !m_player->cInput->right)
	{
		m_player->cTransform->velocity.x = -1 * config.player.speed;
	}
	if (m_player->cInput->right && !m_player->cInput->left)
	{
		m_player->cTransform->velocity.x = config.player.speed;
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
					exp->cShape = std::make_shared<CShape>(12, config.enemy.radius * 5, (Color) {255, 75, 10, 255}, config.bullet.o_col, 0);
					exp->cCollision = std::make_shared<CCollision>(config.enemy.radius * 5);
					exp->cDuration = std::make_shared<CDuration>(config.window.fps / 2, m_currentFrame);
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
	int spawnRate = 3000 - 500 * (m_currentFrame / config.window.fps / 30);
	int maxRate = 100;
	if ( int(m_currentFrame * 1000 * config.enemy.spawn / config.window.fps) % ((spawnRate > maxRate) ? spawnRate : maxRate) == 0)
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
		back.entities.clear();
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
		if ( m_overlay.getPage(0)->isActive() )
		{
			back.step();
			back.spawner();
			for (auto e : back.entities.getEntities())
			{
				if (e->cTransform)
				{
					float posUpdateX = e->cTransform->velocity.x * (1.0f / (float)(config.window.fps));
					float posUpdateY = e->cTransform->velocity.y * (1.0f / (float)(config.window.fps));
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
					e->cTransform->rotation += 90.0 / (float) config.window.fps;
				}
			}
		}
		back.draw();
		// C style string fuckery
		char scoreText[28] = "SCORE: ";
		char scoreNum[21];
		sprintf(scoreNum, "%d", m_score);
		strcat(scoreText, scoreNum);
		Vector2 scorePos = (Vector2) {8, config.font.size + 2};
		char highScoreText[33] = "HIGH SCORE: ";
		char highScoreNum[21];
		sprintf(highScoreNum, "%d", m_highScore);
		strcat(highScoreText, highScoreNum);
		Vector2 highScorePos = (Vector2) {8, config.font.size * 2 + 2};
		char timeText[28] = "TIME: ";
		char timeNum[21];
		sprintf(timeNum, "%d", m_currentFrame / config.window.fps);
		strcat(timeText, timeNum);
		Vector2 timePos = (Vector2) {GetScreenWidth() - 8 * config.font.size,  config.font.size + 2};
		DrawTextEx(config.font.style, scoreText, scorePos, config.font.size, 2, config.font.col);
		DrawTextEx(config.font.style, highScoreText, highScorePos, config.font.size, 2, config.font.col);
		DrawTextEx(config.font.style, timeText, timePos, config.font.size, 2, config.font.col);
		for (auto e : m_entities.getEntities())
		{
			if (e->cTransform)
			{
				if (!m_paused)
				{
					float posUpdateX = e->cTransform->velocity.x * (1.0f / (float)(config.window.fps));
					float posUpdateY = e->cTransform->velocity.y * (1.0f / (float)(config.window.fps));
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
					e->cTransform->rotation += 90.0 / (float) config.window.fps;
				}
				if (e->cLabel)
				{
					DrawTextEx(config.font.style, e->cLabel->text, e->cTransform->pos, e->cLabel->size, 2, e->cLabel->colour);
				}
			}
		}
		if ( m_paused )
		{
			m_overlay.update();
			m_overlay.render();
			DrawTexture(*(m_logo.get()), config.window.width / 2 - m_logo->width / 2, 2, WHITE); // TODO: draw logo with NoGUI
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
	label->cLabel = std::make_shared<CLabel>(labelText, config.font.size * 4, config.font.col);
	Vector2 labelBounds = MeasureTextEx(config.font.style, label->cLabel->text,  config.font.size * 2, 2);
	label->cTransform = std::make_shared<CTransform>((Vector2) {(center.x - labelBounds.x), (center.y - labelBounds.y)});
	label->cDuration = std::make_shared<CDuration>(3 * config.window.fps / config.enemy.spawn, m_currentFrame);
	// create the player
	m_player = m_entities.addEntity("Player");
	m_player->cCollision = std::make_shared<CCollision>(config.player.c_radius);
	m_player->cInput = std::make_shared<CInput>();
	m_player->cShape = std::make_shared<CShape>(config.player.sides, config.player.radius, config.player.fill, config.player.o_col, config.player.o_thick);
	m_player->cTransform = std::make_shared<CTransform>((Vector2) {(center.x - m_player->cShape->radius / 2), (center.y - m_player->cShape->radius / 2)});
	// TODO: settings in config??
	m_player->cDash = std::make_shared<CDash>(config.window.fps / 4, 0, config.window.fps, 2.0, false);
}

void Game::spawnEnemy()
{
	auto enemy = m_entities.addEntity("Enemy");
	float diameter = config.enemy.radius * 2;
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
	colourDiff += abs(fill.r - config.window.col.r);
	colourDiff += abs(fill.g - config.window.col.g);
	colourDiff += abs(fill.b - config.window.col.b);
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
	float randNum1 = static_cast <float> (rand());
	float randNum2 = static_cast <float> (rand());
	float divisor = static_cast <float> (RAND_MAX/ (config.enemy.speed * 2));
	Vector2 vel = (Vector2){randNum1 / divisor - config.enemy.speed, randNum2 / divisor - config.enemy.speed};
	//Vector2 vel = (Vector2) {static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/config.enemy.speed)), static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/config.enemy.speed))};
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
	enemy->cShape = std::make_shared<CShape>(GetRandomValue(3, 8), config.enemy.radius, fill, config.enemy.o_col, config.enemy.o_thick);
	enemy->cCollision =  std::make_shared<CCollision>(config.enemy.c_radius);
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
		e->cCollision = std::make_shared<CCollision>(config.enemy.c_radius / enemy->cShape->sides);
		e->cScore = std::make_shared<CScore>(enemy->cScore->val * 2);
		e->cDuration = std::make_shared<CDuration>(config.enemy.d_life, m_currentFrame);
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
	direction.x = direction.x / mag * config.bullet.speed;
	direction.y = direction.y / mag * config.bullet.speed;
	b->cTransform = std::make_shared<CTransform>(origin, direction);
	b->cShape = std::make_shared<CShape>(10, config.bullet.radius, config.bullet.col, config.bullet.o_col, config.bullet.o_thick);
	b->cCollision = std::make_shared<CCollision>(config.enemy.c_radius);
	b->cDuration = std::make_shared<CDuration>(config.bullet.duration, m_currentFrame);
}

void Game::spawnSpecial()
{
	// TODO: bomb setings in config file?
	int lastCreated = -1 * config.window.fps;
	for (auto e : m_entities.getEntities("Bomb"))
	{
		if (e->cDuration->frameCreated > lastCreated)
		{
			lastCreated = e->cDuration->frameCreated;
		}
	}
	if (m_currentFrame > lastCreated + config.window.fps / 2)
	{
		auto b = m_entities.addEntity("Bomb");
		b->cTransform = std::make_shared<CTransform>(m_player->cTransform->pos);
		b->cShape = std::make_shared<CShape>(4, config.bullet.radius, config.bullet.col, config.bullet.o_col, config.bullet.o_thick);
		b->cDuration =  std::make_shared<CDuration>(config.bullet.duration, m_currentFrame);
	}
}

void Game::sCollision()
{
	// Player
	// horizontal window bounds
	if (m_player->cTransform->pos.x - m_player->cCollision->radius <= 0)
	{
		m_player->cTransform->velocity.x = config.player.speed;
	}
	else if (m_player->cTransform->pos.x + m_player->cCollision->radius > GetScreenWidth())
	{
		m_player->cTransform->velocity.x = -1 * config.player.speed;
	}
	// vertical window bounds
	if (m_player->cTransform->pos.y - m_player->cCollision->radius <= 0)
	{
		m_player->cTransform->velocity.y = config.player.speed;
	}
	else if (m_player->cTransform->pos.y + m_player->cCollision->radius > GetScreenHeight())
	{
		m_player->cTransform->velocity.y = -1 * config.player.speed;
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
	float posx = config.window.width / 2;
	Vector2 radius = {config.window.width / 6, config.window.height / 20};
	
	if ( m_logo )
	{
		UnloadTexture(*(m_logo.get()));
	}
	m_overlay.clear();
	
	NoGUI::Style playStyle = {OFFGRAY, BLACK, (Vector2){posx, config.window.height / 2 + radius.y + 10}, radius, 4, 5, 0};
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
	float posx = config.window.width / 1.5;
	Vector2 radius = {config.window.width / 6, config.window.height / 20};
	
	NoGUI::Style resStyle = {INVISIBLE, WHITE, (Vector2){posx, config.window.height / 2}, radius, 4, 5, 0};
	
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
	back.currCol = config.window.col;
	m_paused = p;
	m_overlay.getPage(1)->setActive(m_paused);
}

void Game::cleanup()
{
	UnloadFont(config.font.style); // we use smart pointers for everything else
	UnloadTexture(*(m_logo.get()));
}
