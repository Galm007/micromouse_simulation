#pragma once

#include <raylib.h>
#include <raylib-cpp.hpp>

#include "maze.h"
#include "point.h"

namespace ray = raylib;

class Mouse {
public:
	ray::Vector2 position;
	float angle;

	Mouse(Maze* maze, Point starting_coord);
	~Mouse();

	ray::Vector2 Forward();
	ray::Vector2 Right();

	void Move(float amount);

	void Draw();
};
