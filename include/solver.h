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

public:
	Solver(Maze* maze, Point starting_coord);
	~Solver();

	void Reset(Point starting_coord);
	void UpdateWalls();
	void Floodfill();
	void Step();

	void Draw(ray::Vector2 pos);
};
