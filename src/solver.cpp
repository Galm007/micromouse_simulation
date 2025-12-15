#include "solver.h"
#include <algorithm>
#include <cstring>
#include <raylib.h>
#include <raygui.h>
#include <queue>
#include <string>
#include <tuple>

Solver::Solver(Maze* maze, Point starting_coord) {
	this->floodfill_maze.position = maze->position;
	this->maze = maze;
	this->starting_coord = starting_coord;
	Reset();
}

Solver::~Solver() {

}

// Reset the conditions to where the solver does not know anything about the maze
void Solver::Reset() {
	coord = starting_coord;
	target_coords = { Point(7, 7), Point(8, 7), Point(7, 8), Point(8, 8) };
	floodfill_maze.Clear();

	memset(visited_coords, 0, sizeof(visited_coords));
	visited_coords[coord.y][coord.x] = true;

	UpdateWalls();
	Floodfill();
}

// Update knowledge about existing walls based on current location
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
	// Reset manhattan distances
	for (int i = 0; i < MAZE_ROWS; i++) {
		for (int j = 0; j < MAZE_COLS; j++) {
			manhattan_dist[i][j] = -1;
		}
	}

	// Floodfill algorithm
	std::queue<std::tuple<Point, int>> q;
	for (Point p : target_coords) {
		q.push(std::make_tuple(p, 0));
		manhattan_dist[p.y][p.x] = 0;
	}

	while (!q.empty()) {
		Point ff_coord = std::get<0>(q.front());
		int next_dist = std::get<1>(q.front()) + 1;
		q.pop();

		if (ff_coord.y > 0 // Check top wall
			&& !floodfill_maze.HWallAt(ff_coord)
			&& manhattan_dist[ff_coord.y - 1][ff_coord.x] == -1) {
			manhattan_dist[ff_coord.y - 1][ff_coord.x] = next_dist;
			q.push(std::make_tuple(ff_coord - Point(0, 1), next_dist));
		}
		if (ff_coord.y < MAZE_ROWS - 1 // Check bottom wall
			&& !floodfill_maze.HWallAt(ff_coord + Point(0, 1))
			&& manhattan_dist[ff_coord.y + 1][ff_coord.x] == -1) {
			manhattan_dist[ff_coord.y + 1][ff_coord.x] = next_dist;
			q.push(std::make_tuple(ff_coord + Point(0, 1), next_dist));
		}
		if (ff_coord.x > 0 // Check left wall
			&& !floodfill_maze.VWallAt(ff_coord)
			&& manhattan_dist[ff_coord.y][ff_coord.x - 1] == -1) {
			manhattan_dist[ff_coord.y][ff_coord.x - 1] = next_dist;
			q.push(std::make_tuple(ff_coord - Point(1, 0), next_dist));
		}
		if (ff_coord.x < MAZE_COLS - 1 // Check right wall
			&& !floodfill_maze.VWallAt(ff_coord + Point(1, 0))
			&& manhattan_dist[ff_coord.y][ff_coord.x + 1] == -1) {
			manhattan_dist[ff_coord.y][ff_coord.x + 1] = next_dist;
			q.push(std::make_tuple(ff_coord + Point(1, 0), next_dist));
		}
	}
}

bool Solver::TargetReached() {
	return std::find(target_coords.begin(), target_coords.end(), coord) != target_coords.end();
}

int Solver::Step() {
	int next_dist = manhattan_dist[coord.y][coord.x] - 1;
	bool moved = false;

	// If there is a path that it can go to, but that path has already been
	// explored, ignore it first. Then on the second loop if no other paths
	// exist, its fine to take it.
	for (int i = 0; i < 2; i++) {
		if (coord.y > 0 // Check top cell
			&& !floodfill_maze.HWallAt(coord)
			&& manhattan_dist[coord.y - 1][coord.x] == next_dist
			&& (moved || !visited_coords[coord.y - 1][coord.x])) {
			coord.y--;
			break;
		} else if (coord.y < MAZE_ROWS - 1 // Check bottom cell
			&& !floodfill_maze.HWallAt(coord + Point(0, 1))
			&& manhattan_dist[coord.y + 1][coord.x] == next_dist
			&& (moved || !visited_coords[coord.y + 1][coord.x])) {
			coord.y++;
			break;
		} else if (coord.x > 0 // Check left cell
			&& !floodfill_maze.VWallAt(coord)
			&& manhattan_dist[coord.y][coord.x - 1] == next_dist
			&& (moved || !visited_coords[coord.y][coord.x - 1])) {
			coord.x--;
			break;
		} else if (coord.x < MAZE_COLS - 1 // Check right cell
			&& !floodfill_maze.VWallAt(coord + Point(1, 0))
			&& manhattan_dist[coord.y][coord.x + 1] == next_dist
			&& (moved || !visited_coords[coord.y][coord.x + 1])) {
			coord.x++;
			break;
		} else {
			moved = true;
		}
	}

	visited_coords[coord.y][coord.x] = true;

	UpdateWalls();
	Floodfill();

	return TargetReached();
}

void Solver::Draw(ray::Vector2 pos, bool show_manhattan_dist) {
	// Draw known walls
	floodfill_maze.position = pos;
	floodfill_maze.Draw(BLACK, ColorAlpha(BLACK, 0.0f));

	// Draw current coord
	DrawCircleV(
		pos + (coord.ToVec2() + ray::Vector2(0.5f, 0.5f)) * MAZE_CELL_SIZE,
		MAZE_CELL_SIZE * 0.4f,
		ORANGE
	);

	// Show manhattan distance of each cell from the goal
	if (show_manhattan_dist) {
		for (int i = 0; i < MAZE_ROWS; i++) {
			for (int j = 0; j < MAZE_COLS; j++) {
				Vector2 p = pos + ray::Vector2(j, i) * MAZE_CELL_SIZE;
				GuiLabel(
					ray::Rectangle(p.x + 10.0f, p.y, 50.0f, 50.0f),
					std::to_string(manhattan_dist[i][j]).c_str()
				);
			}
		}
	}
}
