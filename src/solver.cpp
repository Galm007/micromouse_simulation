#include <algorithm>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <raylib.h>
#include <raygui.h>
#include <queue>
#include <stack>
#include <string>
#include <tuple>

#include "solver.h"
#include "direction.h"
#include "console.h"

#define FOREACH_EDGE(code) \
	for (int row = 0; row <= MAZE_ROWS; row++) {\
		for (int col = 0; col <= MAZE_COLS; col++) {\
			for (int horizontal = 0; horizontal < 2; horizontal++)\
			{\
				code\
			}\
		}\
	}\

void GetEdgesOfCell(bool dest_horizontals[4], Point dest_edge_coords[4], Point coordinate) {
	dest_horizontals[0] = true;
	dest_edge_coords[0] = coordinate;

	dest_horizontals[1] = true;
	dest_edge_coords[1] = coordinate + Point(0, 1);

	dest_horizontals[2] = false;
	dest_edge_coords[2] = coordinate;

	dest_horizontals[3] = false;
	dest_edge_coords[3] = coordinate + Point(1, 0);
}

// Update which edges have been visited based on current location
void Solver::UpdateVisitedEdges() {
	edges[true][coord.y][coord.x].visited = true;
	edges[true][coord.y + 1][coord.x].visited = true;
	edges[false][coord.y][coord.x].visited = true;
	edges[false][coord.y][coord.x + 1].visited = true;
}

// Update knowledge about existing walls based on current location
void Solver::UpdateWalls() {
	if (maze->WallAt(true, coord)) {
		known_maze.SetWalls(coord, coord + Point(1, 0), true);
	}
	if (maze->WallAt(true, coord + Point(0, 1))) {
		known_maze.SetWalls(coord + Point(0, 1), coord + Point(1, 1), true);
	}
	if (maze->WallAt(false, coord)) {
		known_maze.SetWalls(coord, coord + Point(0, 1), true);
	}
	if (maze->WallAt(false, coord + Point(1, 0))) {
		known_maze.SetWalls(coord + Point(1, 0), coord + Point(1, 1), true);
	}
}

// For a diagonal solver, flood starts from player
void Solver::Floodfill(bool visited_edges_only) {
	// Reset floodfill values and directions
	FOREACH_EDGE(
		edges[horizontal][row][col].ff_val = -1.0f;
		edges[horizontal][row][col].dir = DIR_UNKNOWN;
		edges[horizontal][row][col].same_dir = 0;
	);

	// Populate queue with edges of target cells
	std::queue<std::tuple<bool, Point>> q;
	for (Point target : target_coords) {
		bool horizontals[4];
		Point edge_coords[4];
		GetEdgesOfCell(horizontals, edge_coords, target);

		for (int i = 0; i < 4; i++) {
			bool h = horizontals[i];
			Point e = edge_coords[i];
			if (!maze->WallAt(h, e)) {
				edges[h][e.y][e.x].ff_val = 0.0f;
				edges[h][e.y][e.x].dir = (Direction)(1 + 3 * i);
				q.push(std::make_tuple(h, e));
			}
		}
	}

	while (!q.empty()) {
		bool horizontal = std::get<0>(q.front());
		Point edge_coord = std::get<1>(q.front());
		q.pop();

		// Prioritize making edges that share a common direction
		int common_test_dir_i = 0;
		while (common_test_dir_i != -1) {
			common_test_dir_i = -1;

			Edge edge = edges[horizontal][edge_coord.y][edge_coord.x];
			Direction normalized_dir = NormalizeDir(edge.dir);

			// Find the other 3 edges of the cell to evaluate, based on the direction
			bool test_horizontals[3];
			Point test_edge_coords[3];
			GetNextPossibleEdges(test_horizontals, test_edge_coords, normalized_dir);

			// Evaluate each test edge
			for (int i = 0; i < 3; i++) {
				Direction new_dir = (Direction)(normalized_dir + i);
				Point new_coord = edge_coord + test_edge_coords[i];
				Edge& new_edge = edges[test_horizontals[i]][new_coord.y][new_coord.x];

				bool within_bounds = (normalized_dir == DIR_UP && new_coord.y >= 0)
					|| (normalized_dir == DIR_DOWN && new_coord.y < MAZE_ROWS)
					|| (normalized_dir == DIR_LEFT && new_coord.x >= 0)
					|| (normalized_dir == DIR_RIGHT && new_coord.x < MAZE_COLS);

				if (within_bounds
					&& (!visited_edges_only || (visited_edges_only && new_edge.visited))
					&& !maze->WallAt(test_horizontals[i], new_coord)
					&& new_edge.ff_val < 0.0f) {
					// Set edge values and push it to queue
					new_edge.ff_val = edge.ff_val + (new_dir == normalized_dir ? 1.0f : 0.71f);
					new_edge.dir = new_dir;
					q.push(std::make_tuple(test_horizontals[i], new_coord));

					// If they share a common direction, remember it
					if (SimilarDirections(new_edge.dir, edge.dir)) {
						new_edge.same_dir = edge.same_dir + 1;
						common_test_dir_i = i;
					}
				}
			}

			if (common_test_dir_i != -1) {
				// Continue going in the shared common direction
				edge_coord += test_edge_coords[common_test_dir_i];
				horizontal = test_horizontals[common_test_dir_i];
			}
		}
	}
}

void Solver::UpdateSolution() {
	solution = { DIR_UNKNOWN };

	Point edge_coord;
	bool horizontal;
	Edge edge;

	// Find starting edge
	edge.ff_val = 10000.0f;
	bool horizontals[4];
	Point edge_coords[4];
	GetEdgesOfCell(horizontals, edge_coords, coord);
	for (int i = 0; i < 4; i++) {
		Edge new_edge = edges[horizontals[i]][edge_coords[i].y][edge_coords[i].x];
		if (new_edge.ff_val > 0.0f && new_edge.ff_val < edge.ff_val) {
			edge_coord = edge_coords[i];
			horizontal = horizontals[i];
			edge = new_edge;
			solution[0] = (Direction)(1 + 3 * i);
		}
	}

	// Keep going until there are no more edges to go to
	bool moved = true;
	while (moved) {
		moved = false;

		// Add to path
		solution.push_back(ReverseDir(edge.dir));

		// Find the other 3 edges of the cell to test
		Direction normalized_dir = ReverseDir(NormalizeDir(edge.dir));
		GetNextPossibleEdges(horizontals, edge_coords, normalized_dir);

		Point from_edge_coord = edge_coord;
		Edge from_edge = edge;

		// Do two passes:
		// - First pass prioritizes continuing in the same direction.
		// - Otherwise, second pass takes the edge with the least floodfill value.
		for (int j = 0; j < 2 && !moved; j++) {
			// Evaluate each test edge
			for (int i = 0; i < 3; i++) {
				Point new_coord = from_edge_coord + edge_coords[i];

				bool within_bounds = (normalized_dir == DIR_UP && new_coord.y >= 0)
					|| (normalized_dir == DIR_DOWN && new_coord.y < MAZE_ROWS)
					|| (normalized_dir == DIR_LEFT && new_coord.x >= 0)
					|| (normalized_dir == DIR_RIGHT && new_coord.x < MAZE_COLS);

				Edge new_edge = edges[horizontals[i]][new_coord.y][new_coord.x];
				if (within_bounds
					&& new_edge.ff_val >= 0.0f
					&& new_edge.ff_val < from_edge.ff_val
					&& ((j == 0 && SimilarDirections(from_edge.dir, new_edge.dir))
						|| (j == 1 && new_edge.ff_val < edge.ff_val))) {
					edge_coord = new_coord;
					edge = new_edge;
					horizontal = horizontals[i];
					moved = true;
				}
			}
		}
	}
}

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

	FOREACH_EDGE(edges[horizontal][row][col].visited = false;);
	UpdateVisitedEdges();
	UpdateWalls();
	Floodfill(false);
	UpdateSolution();
}

// bool Solver::IsInTarget(Point coordinate) {
// 	return std::find(target_coords.begin(), target_coords.end(), coordinate) != target_coords.end();
// }

// Point Solver::Step() {
// 	// If there is a path that it can go to, but that path has already been
// 	// explored, ignore it first. Then on the second loop if no other paths
// 	// exist, its fine to take it.
// 	// bool moved = false;
// 	// for (int i = 0; i < 2; i++) {
// 	// }
//
// 	visited_edges[coord.y][coord.x] = true;
//
// 	UpdateWalls();
// 	Floodfill(false);
//
// 	return IsInTarget(coord);
// }

void Solver::Draw(ray::Vector2 pos, bool show_floodfill_vals, Font floodfill_font) {
	// Draw known walls
	known_maze.position = pos;
	known_maze.Draw(BLACK, ColorAlpha(BLACK, 0.0f));

	// Draw current coord
	DrawCircleV(maze->CellToPos(coord), MAZE_CELL_SIZE * 0.4f, ORANGE);

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

	// Show path
	ray::Vector2 from = maze->CellToPos(coord) + DirToTransform(solution[0]) * MAZE_CELL_SIZE * 0.5f;
	for (int i = 1; i < solution.size(); i++) {
		ray::Vector2 to = from + DirToTransform(solution[i]) * MAZE_CELL_SIZE;
		DrawLineEx(from, to, 2.0f, PURPLE);
		DrawCircleLinesV(to, 5.0f, PURPLE);
		from = to;
	}

	// Show manhattan distance of each cell from the goal
	if (!show_floodfill_vals) {
		return;
	}

	const int TEXT_HEIGHT = 13;
	int default_text_size = GuiGetStyle(DEFAULT, TEXT_SIZE);
	int default_text_clr = GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL);

	GuiSetFont(floodfill_font);

	for (int i = 0; i < MAZE_ROWS; i++) {
		for (int j = 0; j < MAZE_COLS; j++) {
			char buffer[16];
			float x, y, ff_val;
			Vector2 p = maze->CornerToPos(Point(j, i));

			// Horizontal floodfill values
			if ((ff_val = edges[true][i][j].ff_val) > 0.0f) {
				snprintf(
					buffer,
					sizeof(buffer),
					"%.1f\n%s,%d",
					ff_val,
					DirToStr(edges[true][i][j].dir).c_str(),
					edges[true][i][j].same_dir
				);

				x = p.x + (MAZE_CELL_SIZE - GuiGetTextWidth(buffer)) / 2.0f;
				y = p.y - TEXT_HEIGHT / 2.0f;

				GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(RED));

				GuiSetStyle(DEFAULT, TEXT_SIZE, TEXT_HEIGHT);
				GuiLabel(ray::Rectangle(x, y, 50.0f, TEXT_HEIGHT), buffer);
			}

			// Vertical floodfill values
			if ((ff_val = edges[false][i][j].ff_val) > 0.0f) {
				snprintf(
					buffer,
					sizeof(buffer),
					"%.1f\n%s,%d",
					ff_val,
					DirToStr(edges[false][i][j].dir).c_str(),
					edges[false][i][j].same_dir
				);

				x = p.x - GuiGetTextWidth(buffer) / 2.0f;
				y = p.y + (MAZE_CELL_SIZE - TEXT_HEIGHT) / 2.0f;

				GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(BLUE));

				GuiSetStyle(DEFAULT, TEXT_SIZE, TEXT_HEIGHT);
				GuiLabel(ray::Rectangle(x, y, 50.0f, TEXT_HEIGHT), buffer);
			}
		}
	}

	GuiSetFont(GetFontDefault());
	GuiSetStyle(DEFAULT, TEXT_SIZE, default_text_size);
	GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, default_text_clr);
}
