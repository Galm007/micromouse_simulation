#pragma once

#include "maze.h"
#include "point.h"
#include "direction.h"

struct Edge {
	bool visited = false;
	Direction dir = DIR_UNKNOWN;
	float ff_val = -1.0f;
	int same_dir = 0;
};

class Solver {
private:
	Point coord = Point(0, 0);
	Maze known_maze = Maze(ray::Vector2(0.0f, 0.0f));
	Maze* maze;

	Edge edges[2][MAZE_ROWS + 1][MAZE_COLS + 1] = { }; // edges[horizontal][row][column]
	std::vector<Direction> solution;

	void UpdateVisitedEdges();
	void UpdateWalls();
	void Floodfill(bool visited_edges_only);
	void UpdateSolution();

public:
	std::vector<Point> target_coords;
	Point starting_coord = Point(0, 0);

	Solver(Maze* maze, Point starting_coord);
	~Solver();

	void Reset();
	Point Step();
	void Draw(ray::Vector2 pos, bool show_floodfill_vals, Font floodfill_font);
};
