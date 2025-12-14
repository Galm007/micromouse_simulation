#pragma once

#include <string>
#include <raylib.h>
#include <raylib-cpp.hpp>

#include "point.h"

namespace ray = raylib;

#define MAZE_ROWS 16
#define MAZE_COLS 16
#define MAZE_CELL_SIZE 40

class Maze {
private:
	bool horizontal_walls[MAZE_ROWS + 1][MAZE_COLS];
	bool vertical_walls[MAZE_ROWS][MAZE_COLS + 1];
	
public:
	ray::Vector2 position;

	Maze(Vector2 position);
	~Maze();

	bool Contains(Vector2 pos);
	Point ClosestCornerTo(Vector2 pos);
	Vector2 CornerToPos(Point coord);

	static bool IsWallValid(Point from_corner, Point to_corner);
	void SetWalls(Point from_corner, Point to_corner, bool state);
	bool HWallAt(Point corner);
	bool VWallAt(Point corner);
	void Clear();

	void Draw(Color wall_clr, Color dot_clr);

	int SaveToFile(std::string filename, Point starting_coord);
	int LoadFromFile(std::string filename, Point* starting_coord);
};
