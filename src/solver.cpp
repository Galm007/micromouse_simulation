#include "solver.h"
#include "raylib.h"
#include <iostream>

Solver::Solver(Maze* maze, Point starting_coord) {
	this->coord = starting_coord;
	this->dir = Point(0, 1);
	this->floodfill_maze.position = maze->position;
	this->maze = maze;
}

Solver::~Solver() {

}

void Solver::Reset(Point starting_coord) {
	coord = starting_coord;
	floodfill_maze.Clear();
}

void Solver::UpdateWalls() {
	if (maze->HWallAt(coord)) {
		floodfill_maze.SetWalls(coord, coord + Point(1, 0), true);
	}
	if (maze->HWallAt(coord + Point(0, 1))) {
		floodfill_maze.SetWalls(coord + Point(0, 1), coord + Point(1, 1), true);
	}
	if (maze->VWallAt(coord)) {
		floodfill_maze.SetWalls(coord, coord + Point(0, 1), true);
	}
	if (maze->VWallAt(coord + Point(1, 0))) {
		floodfill_maze.SetWalls(coord + Point(1, 0), coord + Point(1, 1), true);
	}
}

void Solver::Draw() {
	DrawCircleV(
		maze->position + (coord.ToVec2() + ray::Vector2(0.5f, 0.5f)) * MAZE_CELL_SIZE,
		MAZE_CELL_SIZE * 0.4f,
		ORANGE
	);

	floodfill_maze.Draw(BLACK, ColorAlpha(BLACK, 0.0f));
}
