#pragma once
struct Vector2
{
	Vector2() { x = 0; y = 0; }
	Vector2(float xx) { x = xx; y = xx; }
	Vector2(float xx, float yy) { x = xx; y = yy; }
	float x, y;
};

struct Vector3
{
	Vector3() { x = 0; y = 0; z = 0;}
	Vector3(float xx) { x = xx; y = xx; z = xx; }
	Vector3(float xx, float yy, float zz) { x = xx; y = yy; z = zz; }
	float x, y, z;
};