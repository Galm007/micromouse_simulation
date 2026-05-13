#pragma once

#include "maze.hpp"
#include "point.hpp"

class Solver {
public:
	Point starting_coord = Point(0, 0);

	virtual ~Solver() = default;
	virtual void Reset() = 0;
	virtual void SoftReset() = 0;
	virtual void Step() = 0;
	virtual bool IsFinished() = 0;
	virtual void Draw(ray::Vector2 pos, bool show_floodfill_vals, Font floodfill_font) = 0;
};
