#include <algorithm>
#include <raylib.h>
#include <raygui.h>
#include <queue>

#include "solver.h"
#include "direction.h"

#define FOREACH_EDGE(code) \
	for (int row = 0; row <= MAZE_ROWS; row++) {\
		for (int col = 0; col <= MAZE_COLS; col++) {\
			for (int horizontal = 0; horizontal < 2; horizontal++)\
			{\
				Edge& edge = edges[horizontal][row][col];\
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

// Update knowledge about existing walls based on current location
void Solver::UpdateVisited() {
	edges[true][coord.y][coord.x].visited = true;
	edges[true][coord.y + 1][coord.x].visited = true;
	edges[false][coord.y][coord.x].visited = true;
	edges[false][coord.y][coord.x + 1].visited = true;

	if (maze->WallAt(true, coord)) {
		edges[true][coord.y][coord.x].wall_exists = true;
	}
	if (maze->WallAt(true, coord + Point(0, 1))) {
		edges[true][coord.y + 1][coord.x].wall_exists = true;
	}
	if (maze->WallAt(false, coord)) {
		edges[false][coord.y][coord.x].wall_exists = true;
	}
	if (maze->WallAt(false, coord + Point(1, 0))) {
		edges[false][coord.y][coord.x + 1].wall_exists = true;
	}
}

// For a diagonal solver, flood starts from player
void Solver::Floodfill(bool visited_edges_only) {
	// Reset floodfill values and directions
	FOREACH_EDGE(
		edge.ff_val = FF_VAL_FROM_FLOAT(-1.0f);
		edge.same_dir = 0;
		edge.dir = DIR_UNKNOWN;
	);

	// Populate queue with edges of current cell
	std::queue<std::tuple<bool, Point>> q;
	bool horizontals[4];
	Point edge_coords[4];
	GetEdgesOfCell(horizontals, edge_coords, coord);
	for (int i = 0; i < 4; i++) {
		bool h = horizontals[i];
		Point e = edge_coords[i];
		Edge& edge = edges[h][e.y][e.x];
		if (!edge.wall_exists) {
			edge.ff_val = FF_VAL_FROM_FLOAT(0.0f);
			edge.dir = (Direction)(1 + 3 * i);
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
					&& (!visited_edges_only || (visited_edges_only && new_edge.visited))
					&& !new_edge.wall_exists
					&& new_edge.ff_val < FF_VAL_FROM_FLOAT(0.0f)) {
					// Set edge values and push it to queue
					new_edge.ff_val = edge.ff_val + (new_dir == normalized_dir ? 3 : 2);
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

	UpdatePath();
}

void Solver::UpdatePath() {
	path = { };

	Point edge_coord;
	bool horizontal;
	Edge edge;

	// Find starting edge
	edge.ff_val = 0b1111111111;
	bool horizontals[4];
	Point edge_coords[4];
	for (int i = 0; i < target_coords.size(); i++) {
		GetEdgesOfCell(horizontals, edge_coords, target_coords[i]);
		for (int i = 0; i < 4; i++) {
			Edge new_edge = edges[horizontals[i]][edge_coords[i].y][edge_coords[i].x];
			if (new_edge.ff_val >= FF_VAL_FROM_FLOAT(0.0f) && new_edge.ff_val < edge.ff_val) {
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
		path.push_back(std::make_tuple(horizontal, edge_coord));

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
					&& new_edge.ff_val >= FF_VAL_FROM_FLOAT(0.0f)
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

// Get the necessary coordinates to visit to validate a potentially better solution path
void Solver::GetUnvisitedPathCoords() {
	std::vector<Point> unvisited_coords = { };

	Point tmp_coord = coord;

	coord = starting_coord;
	target_coords = { Point(7, 7), Point(8, 7), Point(7, 8), Point(8, 8) };
	Floodfill(false);
	for (int i = 0; i < path.size(); i++) {
		bool horizontal = std::get<0>(path[i]);
		Point edge_coord = std::get<1>(path[i]);
		if (!edges[horizontal][edge_coord.y][edge_coord.x].visited) {
			unvisited_coords.push_back(edge_coord);
		}
	}

	coord = tmp_coord;
	target_coords = unvisited_coords;
	if (target_coords.size() == 0) {
		target_coords = { starting_coord };
	}
}

Solver::Solver(Maze* maze, Point starting_coord) {
	this->maze = maze;
	this->starting_coord = starting_coord;
	Reset();
}

Solver::~Solver() {

}

// Reset the conditions to where the solver does not know anything about the maze
void Solver::Reset() {
	FOREACH_EDGE(
		edge.ff_val = FF_VAL_FROM_FLOAT(-1.0f);
		edge.same_dir = 0;
		edge.wall_exists = false;
		edge.dir = DIR_UNKNOWN;
		edge.visited = false;
	);
	run_number = 0;

	SoftReset();
}

// Get ready for another run, without clearing the solver's knowledge of the maze
void Solver::SoftReset() {
	coord = starting_coord;
	target_coords = { Point(7, 7), Point(8, 7), Point(7, 8), Point(8, 8) };
	finished = false;
	going_back = false;
	run_number++;

	UpdateVisited();
	Floodfill(run_number != 1);
}

void Solver::Step() {
	bool horizontal = std::get<0>(path.back());
	Point edge_coord = std::get<1>(path.back());
	path.pop_back();

	if (maze->WallAt(horizontal, edge_coord)) {
		UpdateVisited();
		if (going_back) {
			GetUnvisitedPathCoords();
		}
		Floodfill(false);

		horizontal = std::get<0>(path.back());
		edge_coord = std::get<1>(path.back());
		path.pop_back();
	}

	// Move and update known walls
	Edge edge = edges[horizontal][edge_coord.y][edge_coord.x];
	coord = DirToCell(edge_coord, edge.dir);
	UpdateVisited();

	auto it = std::find(target_coords.begin(), target_coords.end(), coord);
	if (it != target_coords.end()) {
		// Remove all occurences of coord from the target coords
		do {
			target_coords.erase(it);
		} while ((it = std::find(target_coords.begin(), target_coords.end(), coord)) != target_coords.end());

		if (target_coords.size() == 0) {
			if (going_back) {
				if (coord == starting_coord) {
					// After reaching the starting coord again
					finished = true;
					target_coords = { Point(7, 7), Point(8, 7), Point(7, 8), Point(8, 8) };
				} else {
					// After exploring another possible path
					target_coords = { starting_coord };
				}
			} else {
				// After surveying the goal area
				going_back = true;
				GetUnvisitedPathCoords();
			}
		}

		Floodfill(false);
	}
}

bool Solver::IsFinished() {
	return finished;
}

void Solver::DrawPath(Color clr) {
	ray::Vector2 from = maze->CellToPos(coord);
	for (int i = path.size() - 1; i >= 0; i--) {
		bool horizontal = std::get<0>(path[i]);
		Point edge_coord = std::get<1>(path[i]);
		ray::Vector2 to = maze->CornerToPos(edge_coord) + (horizontal
			? ray::Vector2(MAZE_CELL_SIZE / 2.0f, 0.0f)
			: ray::Vector2(0.0f, MAZE_CELL_SIZE / 2.0f));

		DrawLineEx(from, to, 3.0f, clr);
		from = to;
	}
}

void Solver::Draw(ray::Vector2 pos, bool show_floodfill_vals, Font floodfill_font) {
	// Draw known walls
	FOREACH_EDGE(
		if (edge.visited && edge.wall_exists) {
			ray::Vector2 from = pos + ray::Vector2(col, row) * MAZE_CELL_SIZE;
			ray::Vector2 to = from + (horizontal
				? ray::Vector2(MAZE_CELL_SIZE, 0.0f)
				: ray::Vector2(0.0f, MAZE_CELL_SIZE));
			DrawLineV(from, to, BLACK);
		}
	);

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
		Vector2 p = maze->CornerToPos(Point(i, MAZE_ROWS));
		GuiLabel(
			ray::Rectangle(p.x + 10.0f, p.y, 50.0f, 50.0f),
			std::to_string(i).c_str()
		);
	}

	if (finished) {
		// Solution
		Floodfill(true);
		DrawPath(DARKBLUE);

		// Alternative solution
		Floodfill(false);
		DrawPath(BLACK);
	} else {
		// Current path
		DrawPath(DARKBLUE);
	}

	// Show unvisited coords of a potentially better path
	for (Point target : target_coords) {
		DrawCircleLinesV(maze->CellToPos(target), MAZE_CELL_SIZE * 0.4f, BLACK);
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
			float x, y, ff_val_f;
			Vector2 p = maze->CornerToPos(Point(j, i));

			// Horizontal floodfill values
			if ((ff_val_f = FF_VAL_TO_FLOAT(edges[true][i][j].ff_val)) >= 0.0f) {
				snprintf(
					buffer,
					sizeof(buffer),
					"%.1f\n%s,%d",
					ff_val_f,
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
			if ((ff_val_f = FF_VAL_TO_FLOAT(edges[false][i][j].ff_val)) >= 0.0f) {
				snprintf(
					buffer,
					sizeof(buffer),
					"%.1f\n%s,%d",
					ff_val_f,
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
