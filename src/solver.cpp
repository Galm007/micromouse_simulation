#include <algorithm>
#include <cstdio>
#include <cstring>
#include <raylib.h>
#include <raygui.h>
#include <queue>
#include <string>
#include <tuple>

#include "solver.h"
#include "direction.h"

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
	visited_coords[coord.y][coord.x] = true;
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

	// Populate queue with edges of current cell
	std::queue<std::tuple<bool, Point>> q;
	bool horizontals[4];
	Point edge_coords[4];
	GetEdgesOfCell(horizontals, edge_coords, coord);
	for (int i = 0; i < 4; i++) {
		bool h = horizontals[i];
		Point e = edge_coords[i];
		if (!known_maze.WallAt(h, e)) {
			edges[h][e.y][e.x].ff_val = 0.0f;
			edges[h][e.y][e.x].dir = (Direction)(1 + 3 * i);
			q.push(std::make_tuple(h, e));
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
			GetNextPossibleEdges(horizontals, edge_coords, normalized_dir);
			for (int i = 0; i < 3; i++) {
				Direction new_dir = (Direction)(normalized_dir + i);
				Point new_coord = edge_coord + edge_coords[i];
				Edge& new_edge = edges[horizontals[i]][new_coord.y][new_coord.x];

				bool within_bounds = (normalized_dir == DIR_UP && new_coord.y >= 0)
					|| (normalized_dir == DIR_DOWN && new_coord.y < MAZE_ROWS)
					|| (normalized_dir == DIR_LEFT && new_coord.x >= 0)
					|| (normalized_dir == DIR_RIGHT && new_coord.x < MAZE_COLS);

				if (within_bounds
					&& (!visited_edges_only || (visited_edges_only && visited_coords[new_coord.y][new_coord.x]))
					&& !known_maze.WallAt(horizontals[i], new_coord)
					&& new_edge.ff_val < 0.0f) {
					// Set edge values and push it to queue
					new_edge.ff_val = edge.ff_val + (new_dir == normalized_dir ? 1.0f : 0.71f);
					new_edge.dir = new_dir;
					q.push(std::make_tuple(horizontals[i], new_coord));

					// Keep track of edges that share a common direction
					if (SimilarDirections(new_edge.dir, edge.dir)) {
						new_edge.same_dir = edge.same_dir + 1;
						common_test_dir_i = i;
					}
				}
			}

			if (common_test_dir_i != -1) {
				// Continue going in the same shared common direction
				edge_coord += edge_coords[common_test_dir_i];
				horizontal = horizontals[common_test_dir_i];
			}
		}
		
	}
}

void Solver::UpdateSolution() {
	solution = { };

	Point edge_coord;
	bool horizontal;
	Edge edge;

	// Find starting edge
	edge.ff_val = 10000.0f;
	bool horizontals[4];
	Point edge_coords[4];
	for (int i = 0; i < target_coords.size(); i++) {
		GetEdgesOfCell(horizontals, edge_coords, target_coords[i]);
		for (int i = 0; i < 4; i++) {
			Edge new_edge = edges[horizontals[i]][edge_coords[i].y][edge_coords[i].x];
			if (new_edge.ff_val >= 0.0f && new_edge.ff_val < edge.ff_val) {
				edge_coord = edge_coords[i];
				horizontal = horizontals[i];
				edge = new_edge;
			}
		}
	}

	// Keep going until there are no more edges to go to
	bool moved = true;
	while (moved) {
		moved = false;

		// Add to path
		solution.push_back(std::make_tuple(horizontal, edge_coord));

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
	known_maze.Clear();

	memset(visited_coords, 0, sizeof(visited_coords));
	FOREACH_EDGE(
		edges[horizontal][row][col].ff_val = -1.0f;
		edges[horizontal][row][col].dir = DIR_UNKNOWN;
		edges[horizontal][row][col].same_dir = 0;
	);

	SoftReset();
}

// Get ready for another run, without clearing the solver's knowledge of the maze
void Solver::SoftReset() {
	finished = false;
	target_coords = { Point(7, 7), Point(8, 7), Point(7, 8), Point(8, 8) };

	UpdateVisitedEdges();
	UpdateWalls();
	Floodfill(false);
	UpdateSolution();
}

void Solver::Step() {
	bool horizontal = std::get<0>(solution.back());
	Point edge_coord = std::get<1>(solution.back());
	solution.pop_back();

	if (maze->WallAt(horizontal, edge_coord)) {
		UpdateVisitedEdges();
		UpdateWalls();
		Floodfill(false);
		UpdateSolution();

		horizontal = std::get<0>(solution.back());
		edge_coord = std::get<1>(solution.back());
		solution.pop_back();
	}

	// Move and update known walls
	Edge edge = edges[horizontal][edge_coord.y][edge_coord.x];
	coord = DirToCell(edge_coord, edge.dir);
	UpdateVisitedEdges();
	UpdateWalls();

	auto it = std::find(target_coords.begin(), target_coords.end(), coord);
	if (it != target_coords.end()) {
		target_coords.erase(it);

		if (target_coords.size() == 0) {
			if (coord == starting_coord) {
				finished = true;
				target_coords = { Point(7, 7), Point(8, 7), Point(7, 8), Point(8, 8) };
			} else {
				// Go back to starting position once goal is found
				target_coords = { starting_coord };
			}
		}

		Floodfill(false);
		UpdateSolution();
	}
}

bool Solver::IsFinished() {
	return finished;
}

void Solver::DrawSolution(Color clr) {
	ray::Vector2 from = maze->CellToPos(coord);
	for (int i = solution.size() - 1; i >= 0; i--) {
		bool horizontal = std::get<0>(solution[i]);
		Point edge_coord = std::get<1>(solution[i]);
		ray::Vector2 to = maze->CornerToPos(edge_coord) + (horizontal
			? ray::Vector2(MAZE_CELL_SIZE / 2.0f, 0.0f)
			: ray::Vector2(0.0f, MAZE_CELL_SIZE / 2.0f));

		DrawLineEx(from, to, 2.0f, clr);
		DrawCircleLinesV(to, 5.0f, clr);
		from = to;
	}
}

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

	if (finished) {
		// Solution
		Floodfill(true);
		UpdateSolution();
		DrawSolution(PURPLE);

		// Alternative solution
		Floodfill(false);
		UpdateSolution();
		DrawSolution(BLACK);
	} else {
		// Current path
		DrawSolution(PURPLE);
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
			if ((ff_val = edges[true][i][j].ff_val) >= 0.0f) {
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
			if ((ff_val = edges[false][i][j].ff_val) >= 0.0f) {
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
