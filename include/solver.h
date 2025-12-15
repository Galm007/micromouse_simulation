#pragma once

#include "maze.h"
#include "point.h"

enum SolverState {
	SEARCHING,
	SURVEYING_GOAL_AREA
};

class Solver {
private:
	SolverState state = SEARCHING;
	Point coord = Point(0, 0);
	Maze floodfill_maze = Maze(ray::Vector2(0.0f, 0.0f));
	Maze* maze;

	int manhattan_dist[MAZE_ROWS][MAZE_COLS] = {-1};
	bool visited_coords[MAZE_ROWS][MAZE_COLS] = {false};

public:
	std::vector<Point> target_coords;
	Point starting_coord = Point(0, 0);

	Solver(Maze* maze, Point starting_coord);
	~Solver();

	void Reset();
	void UpdateWalls();
	void Floodfill();
	bool TargetReached();
	int Step();

	void Draw(ray::Vector2 pos, bool show_manhattan_dist);
};
