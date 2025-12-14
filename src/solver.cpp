#include "solver.h"
#include <raylib.h>
#include <raygui.h>
#include <queue>
#include <string>
#include <tuple>

Solver::Solver(Maze* maze, Point starting_coord) {
	this->floodfill_maze.position = maze->position;
	this->maze = maze;
	Reset(starting_coord);
}

Solver::~Solver() {

}

void Solver::Reset(Point starting_coord) {
	coord = starting_coord;
	floodfill_maze.Clear();

	UpdateWalls();
	Floodfill();
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

void Solver::Floodfill() {
	for (int i = 0; i < MAZE_ROWS; i++) {
		for (int j = 0; j < MAZE_COLS; j++) {
			manhattan_dist[i][j] = -1;
		}
	}

	manhattan_dist[7][7] = 0;
	manhattan_dist[7][8] = 0;
	manhattan_dist[8][7] = 0;
	manhattan_dist[8][8] = 0;

	std::queue<std::tuple<Point, int>> q;
	q.push(std::make_tuple(Point(7, 7), 0));
	q.push(std::make_tuple(Point(7, 8), 0));
	q.push(std::make_tuple(Point(8, 7), 0));
	q.push(std::make_tuple(Point(8, 8), 0));

	while (!q.empty()) {
		Point ff_coord = std::get<0>(q.front());
		int next_dist = std::get<1>(q.front()) + 1;
		q.pop();

		if (ff_coord.y > 0
			&& !floodfill_maze.HWallAt(ff_coord)
			&& manhattan_dist[ff_coord.y - 1][ff_coord.x] == -1) {
			manhattan_dist[ff_coord.y - 1][ff_coord.x] = next_dist;
			q.push(std::make_tuple(ff_coord - Point(0, 1), next_dist));
		}
		if (ff_coord.y < MAZE_ROWS - 1
			&& !floodfill_maze.HWallAt(ff_coord + Point(0, 1))
			&& manhattan_dist[ff_coord.y + 1][ff_coord.x] == -1) {
			manhattan_dist[ff_coord.y + 1][ff_coord.x] = next_dist;
			q.push(std::make_tuple(ff_coord + Point(0, 1), next_dist));
		}
		if (ff_coord.x > 0
			&& !floodfill_maze.VWallAt(ff_coord)
			&& manhattan_dist[ff_coord.y][ff_coord.x - 1] == -1) {
			manhattan_dist[ff_coord.y][ff_coord.x - 1] = next_dist;
			q.push(std::make_tuple(ff_coord - Point(1, 0), next_dist));
		}
		if (ff_coord.x < MAZE_COLS - 1
			&& !floodfill_maze.VWallAt(ff_coord + Point(1, 0))
			&& manhattan_dist[ff_coord.y][ff_coord.x + 1] == -1) {
			manhattan_dist[ff_coord.y][ff_coord.x + 1] = next_dist;
			q.push(std::make_tuple(ff_coord + Point(1, 0), next_dist));
		}
	}
}

void Solver::Step() {
	int next_dist = manhattan_dist[coord.y][coord.x] - 1;
	if (coord.y > 0
		&& !floodfill_maze.HWallAt(coord)
		&& manhattan_dist[coord.y - 1][coord.x] == next_dist) {
		coord.y--;
	}
	else if (coord.y < MAZE_ROWS - 1
		&& !floodfill_maze.HWallAt(coord + Point(0, 1))
		&& manhattan_dist[coord.y + 1][coord.x] == next_dist) {
		coord.y++;
	}
	else if (coord.x > 0
		&& !floodfill_maze.VWallAt(coord)
		&& manhattan_dist[coord.y][coord.x - 1] == next_dist) {
		coord.x--;
	}
	else if (coord.x < MAZE_COLS - 1
		&& !floodfill_maze.VWallAt(coord + Point(1, 0))
		&& manhattan_dist[coord.y][coord.x + 1] == next_dist) {
		coord.x++;
	}

	UpdateWalls();
	Floodfill();
}

void Solver::Draw() {
	DrawCircleV(
		maze->position + (coord.ToVec2() + ray::Vector2(0.5f, 0.5f)) * MAZE_CELL_SIZE,
		MAZE_CELL_SIZE * 0.4f,
		ORANGE
	);

	floodfill_maze.Draw(BLACK, ColorAlpha(BLACK, 0.0f));

	for (int i = 0; i < MAZE_ROWS; i++) {
		for (int j = 0; j < MAZE_COLS; j++) {
			Vector2 pos = maze->position + ray::Vector2(j, i) * MAZE_CELL_SIZE;
			GuiLabel(
				ray::Rectangle(pos.x + 10.0f, pos.y, 50.0f, 50.0f),
				std::to_string(manhattan_dist[i][j]).c_str()
			);
		}
	}
}
