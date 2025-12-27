#pragma once

#include <tuple>

#include "maze.h"
#include "point.h"
#include "direction.h"

#define FF_VAL_TO_FLOAT(n) (((n) / 3.0f) - 1.0f)
#define FF_VAL_FROM_FLOAT(x) (((x) + 1.0f) * 3.0f)

struct Edge {
	uint16_t ff_val : 10;
	uint8_t same_dir : 6;
	Direction dir : 7;
	bool visited : 1;
};

class Solver {
private:
	Point coord = Point(0, 0);
	Maze known_maze = Maze(ray::Vector2(0.0f, 0.0f));
	Maze* maze;

	Edge edges[2][MAZE_ROWS + 1][MAZE_COLS + 1] = { 0 }; // edges[horizontal][row][column]
	std::vector<std::tuple<bool, Point>> path;
	bool finished;
	bool going_back;
	int run_number;

	void UpdateVisited();
	void Floodfill(bool visited_edges_only);
	void UpdatePath();
	void DrawPath(Color clr);

	std::vector<Point> GetUnvisitedPathCoords();

public:
	std::vector<Point> target_coords;
	Point starting_coord = Point(0, 0);

	Solver(Maze* maze, Point starting_coord);
	~Solver();

	void Reset();
	void SoftReset();
	void Step();
	bool IsFinished();
	void Draw(ray::Vector2 pos, bool show_floodfill_vals, Font floodfill_font);
};
