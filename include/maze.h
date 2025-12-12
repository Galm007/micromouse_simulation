#pragma once

#include <string>
#include <raylib.h>

#include "point.h"

#define MAZE_ROWS 16
#define MAZE_COLS 16
#define MAZE_CELL_SIZE 40

class Maze {
private:
	bool horizontal_walls[MAZE_ROWS + 1][MAZE_COLS];
	bool vertical_walls[MAZE_ROWS][MAZE_COLS + 1];
	Vector2 position;
	
public:
	Maze(Vector2 position);
	~Maze();

	Vector2 GetPosition();

	bool Contains(Vector2 pos);
	Point ClosestCornerTo(Vector2 pos);
	Vector2 CornerToPos(Point coord);

	static bool IsWallValid(Point from_corner, Point to_corner);
	void AddWall(Point from_corner, Point to_corner);

	void Draw(Color wall_clr, Color dot_clr);

	int SaveToFile(std::string filename);
	int LoadFromFile(std::string filename);
};
