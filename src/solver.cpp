#include <algorithm>
#include <cstring>
#include <raylib.h>
#include <raygui.h>
#include <queue>
#include <stack>
#include <string>
#include <tuple>

#include "solver.h"

Solver::Solver(Maze* maze, Point starting_coord) {
	this->known_maze.position = maze->position;
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
	known_maze.Clear();

	memset(visited_coords, 0, sizeof(visited_coords));
	visited_coords[coord.y][coord.x] = true;

	UpdateWalls();
	Floodfill(false);
}

// Update knowledge about existing walls based on current location
void Solver::UpdateWalls() {
	if (maze->HWallAt(coord)) {
		known_maze.SetWalls(coord, coord + Point(1, 0), true);
	}
	if (maze->HWallAt(coord + Point(0, 1))) {
		known_maze.SetWalls(coord + Point(0, 1), coord + Point(1, 1), true);
	}
	if (maze->VWallAt(coord)) {
		known_maze.SetWalls(coord, coord + Point(0, 1), true);
	}
	if (maze->VWallAt(coord + Point(1, 0))) {
		known_maze.SetWalls(coord + Point(1, 0), coord + Point(1, 1), true);
	}
}

void Solver::Floodfill(bool visited_coords_only) {
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
		Point cell = std::get<0>(q.front());
		int next_dist = std::get<1>(q.front()) + 1;
		q.pop();

		if (cell.y > 0 // Check top wall
			&& !known_maze.HWallAt(cell)
			&& manhattan_dist[cell.y - 1][cell.x] == -1
			&& (!visited_coords_only || (visited_coords_only && visited_coords[cell.y - 1][cell.x]))) {
			manhattan_dist[cell.y - 1][cell.x] = next_dist;
			q.push(std::make_tuple(cell - Point(0, 1), next_dist));
		}
		if (cell.y < MAZE_ROWS - 1 // Check bottom wall
			&& !known_maze.HWallAt(cell + Point(0, 1))
			&& manhattan_dist[cell.y + 1][cell.x] == -1
			&& (!visited_coords_only || (visited_coords_only && visited_coords[cell.y + 1][cell.x]))) {
			manhattan_dist[cell.y + 1][cell.x] = next_dist;
			q.push(std::make_tuple(cell + Point(0, 1), next_dist));
		}
		if (cell.x > 0 // Check left wall
			&& !known_maze.VWallAt(cell)
			&& manhattan_dist[cell.y][cell.x - 1] == -1
			&& (!visited_coords_only || (visited_coords_only && visited_coords[cell.y][cell.x - 1]))) {
			manhattan_dist[cell.y][cell.x - 1] = next_dist;
			q.push(std::make_tuple(cell - Point(1, 0), next_dist));
		}
		if (cell.x < MAZE_COLS - 1 // Check right wall
			&& !known_maze.VWallAt(cell + Point(1, 0))
			&& manhattan_dist[cell.y][cell.x + 1] == -1
			&& (!visited_coords_only || (visited_coords_only && visited_coords[cell.y][cell.x + 1]))) {
			manhattan_dist[cell.y][cell.x + 1] = next_dist;
			q.push(std::make_tuple(cell + Point(1, 0), next_dist));
		}
	}
}

bool Solver::IsInTarget(Point coordinate) {
	return std::find(target_coords.begin(), target_coords.end(), coordinate) != target_coords.end();
}

int Solver::Step() {
	int next_dist = manhattan_dist[coord.y][coord.x] - 1;
	bool moved = false;

	// If there is a path that it can go to, but that path has already been
	// explored, ignore it first. Then on the second loop if no other paths
	// exist, its fine to take it.
	for (int i = 0; i < 2; i++) {
		if (coord.y > 0 // Check top cell
			&& !known_maze.HWallAt(coord)
			&& manhattan_dist[coord.y - 1][coord.x] == next_dist
			&& (moved || !visited_coords[coord.y - 1][coord.x])) {
			coord.y--;
			break;
		} else if (coord.y < MAZE_ROWS - 1 // Check bottom cell
			&& !known_maze.HWallAt(coord + Point(0, 1))
			&& manhattan_dist[coord.y + 1][coord.x] == next_dist
			&& (moved || !visited_coords[coord.y + 1][coord.x])) {
			coord.y++;
			break;
		} else if (coord.x > 0 // Check left cell
			&& !known_maze.VWallAt(coord)
			&& manhattan_dist[coord.y][coord.x - 1] == next_dist
			&& (moved || !visited_coords[coord.y][coord.x - 1])) {
			coord.x--;
			break;
		} else if (coord.x < MAZE_COLS - 1 // Check right cell
			&& !known_maze.VWallAt(coord + Point(1, 0))
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
	Floodfill(false);

	return IsInTarget(coord);
}

// Searches the known maze in parallel using independent "agents"
// Agents:
// - Follow down the manhattan distances
// - Die after meeting a dead end
// - Spawn other agents at junctions
// - Cannot go to a cell that has already been visited by another agent
std::vector<std::vector<Point>> Solver::FindSolutions() {
	std::vector<std::vector<Point>> solutions;
	std::stack<Point> agents;
	std::stack<std::vector<Point>> paths;

	target_coords = { Point(7, 7), Point(8, 7), Point(7, 8), Point(8, 8) };
	Floodfill(true);

	agents.push(starting_coord);
	paths.push({});
	memset(visited_coords, 0, sizeof(visited_coords)); // Reusing this array

	while (!agents.empty()) {
		Point a = agents.top();
		agents.pop();

		while (true) {
			int next_dist = manhattan_dist[a.y][a.x] - 1;
			Point move_dir = Point(0, 0);

			visited_coords[a.y][a.x] = true;
			paths.top().push_back(a);

			if (IsInTarget(a)) {
				solutions.push_back(paths.top());
				break;
			}

			if (a.y > 0
				&& manhattan_dist[a.y - 1][a.x] == next_dist
				&& !known_maze.HWallAt(a)
				&& !visited_coords[a.y - 1][a.x]) {
				if (move_dir == Point(0, 0)) {
					move_dir = Point(0, -1);
				} else {
					agents.push(a - Point(0, 1));
					paths.push(paths.top());
				}
			}
			if (a.y < MAZE_ROWS - 1
				&& manhattan_dist[a.y + 1][a.x] == next_dist
				&& !known_maze.HWallAt(a + Point(0, 1))
				&& !visited_coords[a.y + 1][a.x]) {
				if (move_dir == Point(0, 0)) {
					move_dir = Point(0, 1);
				} else {
					agents.push(a + Point(0, 1));
					paths.push(paths.top());
				}
			}
			if (a.x > 0
				&& manhattan_dist[a.y][a.x - 1] == next_dist
				&& !known_maze.VWallAt(a)
				&& !visited_coords[a.y][a.x - 1]) {
				if (move_dir == Point(0, 0)) {
					move_dir = Point(-1, 0);
				} else {
					agents.push(a - Point(1, 0));
					paths.push(paths.top());
				}
			}
			if (a.x < MAZE_COLS - 1
				&& manhattan_dist[a.y][a.x + 1] == next_dist
				&& !known_maze.VWallAt(a + Point(1, 0))
				&& !visited_coords[a.y][a.x + 1]) {
				if (move_dir == Point(0, 0)) {
					move_dir = Point(1, 0);
				} else {
					agents.push(a + Point(1, 0));
					paths.push(paths.top());
				}
			}

			if (move_dir != Point(0, 0)) {
				a += move_dir;
			} else {
				paths.pop();
				break;
			}
		}
	}

	return solutions;
}

void Solver::Draw(ray::Vector2 pos, bool show_manhattan_dist) {
	// Draw known walls
	known_maze.position = pos;
	known_maze.Draw(BLACK, ColorAlpha(BLACK, 0.0f));

	// Draw current coord
	DrawCircleV(maze->CellToPos(coord), MAZE_CELL_SIZE * 0.4f, ORANGE);

	// Show manhattan distance of each cell from the goal
	if (show_manhattan_dist) {
		for (int i = 0; i < MAZE_ROWS; i++) {
			for (int j = 0; j < MAZE_COLS; j++) {
				int mdist = manhattan_dist[i][j];
				// Don't show for unexplored areas
				if (mdist == -1) {
					continue;
				}

				Vector2 p = maze->CornerToPos(Point(j, i));
				GuiLabel(
					ray::Rectangle(p.x + 10.0f, p.y, 50.0f, 50.0f),
					std::to_string(mdist).c_str()
				);
			}
		}
	}

	// Show row and column labels
	for (int i = 0; i < MAZE_ROWS; i++) {
		Vector2 p = maze->CornerToPos(Point(-1, i));
		GuiLabel(
			ray::Rectangle(p.x + 10.0f, p.y, 50.0f, 50.0f),
			std::to_string(i).c_str()
		);
	}
	for (int i = 0; i < MAZE_COLS; i++) {
		Vector2 p = maze->CornerToPos(Point(i, 16));
		GuiLabel(
			ray::Rectangle(p.x + 10.0f, p.y, 50.0f, 50.0f),
			std::to_string(i).c_str()
		);
	}
}
