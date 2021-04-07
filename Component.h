#pragma once

#include "raylib.h"

class CTransform 
{
public:
	Vector2 pos;
	Vector2 velocity;
	float rotation;
	
	CTransform(const Vector2 & p = Vector2{0.0, 0.0}, const Vector2 & v = Vector2{0, 0}, float a = 0.0)
		: pos(p), velocity(v), rotation(a) {}
};

class CShape
{
public:
	int sides;
	float radius;
	Color colour;
	Color outlineC;
	int outlineW;
	
	CShape(int s, float r, const Color &fill, const Color &outline, float thickness)
		: sides(s), radius(r), colour(fill), outlineC(outline), outlineW(thickness) {}
};

class CCollision
{
public:
	float radius = 0;
	CCollision(float r)
		: radius(r) {}
};

class CInput
{
public:
	bool up = false;
	bool left = false;
	bool right = false;
	bool down = false;
	bool shoot = false;
	bool special = false;
	bool dash = false;
	
	CInput() {}
};

class CScore
{
public:
	int val = 0;
	
	CScore(int s)
		: val(s) {}
};

class CDuration
{
public:
	int frames = 1;
	int frameCreated;
	
	CDuration(int lifespan, int created)
		: frames(lifespan), frameCreated(created) {}
};

class CDash
{
public:
	int frames = 0;
	int frameStarted;
	int delay = 1;
	float speedMod = 1.0;
	bool active = false;
	
	CDash(int duration, int started, int cooldown, float boost, bool dashing)
		: frames(duration), frameStarted(started), delay(cooldown), speedMod(boost), active(dashing) {}
};
