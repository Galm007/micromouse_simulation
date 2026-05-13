#include <algorithm>
#include <cstring>
#include <queue>
#include <raylib.h>
#include <raygui.h>

#include "solver/simple_solver.hpp"
#include "maze.hpp"

// Returns true if a new wall (not seen before) is discovered around current cell
bool SimpleSolver::FindSurroundingWalls() {
	bool new_wall = false;
	cell_visited[coord.y][coord.x] = true;

	if (!h_wall_seen[coord.y][coord.x]) {
		h_wall_seen[coord.y][coord.x] = true;
		if (maze->WallAt(true, coord)) {
			known_h_walls[coord.y][coord.x] = true;
			new_wall = true;
		}
	}
	if (!h_wall_seen[coord.y + 1][coord.x]) {
		h_wall_seen[coord.y + 1][coord.x] = true;
		if (maze->WallAt(true, coord + Point(0, 1))) {
			known_h_walls[coord.y + 1][coord.x] = true;
			new_wall = true;
		}
	}
	if (!v_wall_seen[coord.y][coord.x]) {
		v_wall_seen[coord.y][coord.x] = true;
		if (maze->WallAt(false, coord)) {
			known_v_walls[coord.y][coord.x] = true;
			new_wall = true;
		}
	}
	if (!v_wall_seen[coord.y][coord.x + 1]) {
		v_wall_seen[coord.y][coord.x + 1] = true;
		if (maze->WallAt(false, coord + Point(1, 0))) {
			known_v_walls[coord.y][coord.x + 1] = true;
			new_wall = true;
		}
	}

	return new_wall;
}

// BFS from target_coords outward, assigning Manhattan distance to each reachable cell.
// If visited_cells_only, only cells the robot has physically visited are expanded.
void SimpleSolver::Floodfill(bool visited_cells_only) {
	for (int r = 0; r < MAZE_ROWS; r++)
		for (int c = 0; c < MAZE_COLS; c++)
			ff_vals[r][c] = SIMPLE_FF_UNSET;

	std::queue<Point> q;
	for (Point t : target_coords) {
		if (t.x < 0 || t.x >= MAZE_COLS || t.y < 0 || t.y >= MAZE_ROWS) continue;
		if (visited_cells_only && !cell_visited[t.y][t.x]) continue;
		if (ff_vals[t.y][t.x] != SIMPLE_FF_UNSET) continue;
		ff_vals[t.y][t.x] = 0;
		q.push(t);
	}

	while (!q.empty()) {
		Point p = q.front(); q.pop();
		uint16_t next_val = (uint16_t)(ff_vals[p.y][p.x] + 1);

		auto try_expand = [&](Point n, bool wall) {
			if (wall) return;
			if (ff_vals[n.y][n.x] != SIMPLE_FF_UNSET) return;
			if (visited_cells_only && !cell_visited[n.y][n.x]) return;
			ff_vals[n.y][n.x] = next_val;
			q.push(n);
		};

		if (p.y > 0)             try_expand(Point(p.x, (int8_t)(p.y - 1)), known_h_walls[p.y][p.x]);
		if (p.y < MAZE_ROWS - 1) try_expand(Point(p.x, (int8_t)(p.y + 1)), known_h_walls[p.y + 1][p.x]);
		if (p.x > 0)             try_expand(Point((int8_t)(p.x - 1), p.y), known_v_walls[p.y][p.x]);
		if (p.x < MAZE_COLS - 1) try_expand(Point((int8_t)(p.x + 1), p.y), known_v_walls[p.y][p.x + 1]);
	}

	UpdatePath();
}

// Greedily traces the shortest path from coord toward any target by following
// decreasing floodfill values. Stores path in reverse so path.back() = next step.
void SimpleSolver::UpdatePath() {
	path.clear();

	Point cur = coord;
	int max_steps = MAZE_ROWS * MAZE_COLS;

	while (max_steps-- > 0) {
		for (Point& t : target_coords) {
			if (cur == t) {
				std::reverse(path.begin(), path.end());
				return;
			}
		}

		if (ff_vals[cur.y][cur.x] == SIMPLE_FF_UNSET) break;

		Point best = cur;
		uint16_t best_val = ff_vals[cur.y][cur.x];

		auto try_neighbor = [&](Point n, bool wall) {
			if (wall) return;
			if (ff_vals[n.y][n.x] < best_val) { best = n; best_val = ff_vals[n.y][n.x]; }
		};

		if (cur.y > 0)             try_neighbor(Point(cur.x, (int8_t)(cur.y - 1)), known_h_walls[cur.y][cur.x]);
		if (cur.y < MAZE_ROWS - 1) try_neighbor(Point(cur.x, (int8_t)(cur.y + 1)), known_h_walls[cur.y + 1][cur.x]);
		if (cur.x > 0)             try_neighbor(Point((int8_t)(cur.x - 1), cur.y), known_v_walls[cur.y][cur.x]);
		if (cur.x < MAZE_COLS - 1) try_neighbor(Point((int8_t)(cur.x + 1), cur.y), known_v_walls[cur.y][cur.x + 1]);

		if (best == cur) break;

		path.push_back(best);
		cur = best;
	}

	std::reverse(path.begin(), path.end());
}

// Compute the unvisited cells on the currently optimal path (start -> goal) to visit
// before heading home, so the robot can potentially discover a shorter route.
void SimpleSolver::UpdateTargetCoords() {
	Point tmp_coord = coord;

	coord = starting_coord;
	target_coords = maze->goals;
	Floodfill(false);

	std::vector<Point> unvisited;
	for (Point& p : path) {
		if (!cell_visited[p.y][p.x]) {
			unvisited.push_back(p);
		}
	}

	coord = tmp_coord;
	target_coords = unvisited;
	if (target_coords.empty()) {
		target_coords = { starting_coord };
	}
}

SimpleSolver::SimpleSolver(Maze* maze, Point starting_coord) {
	this->maze = maze;
	this->starting_coord = starting_coord;
	Reset();
}

SimpleSolver::~SimpleSolver() { }

void SimpleSolver::Reset() {
	memset(known_h_walls, 0, sizeof(known_h_walls));
	memset(known_v_walls, 0, sizeof(known_v_walls));
	memset(h_wall_seen,   0, sizeof(h_wall_seen));
	memset(v_wall_seen,   0, sizeof(v_wall_seen));
	memset(cell_visited,  0, sizeof(cell_visited));

	for (int c = 0; c < MAZE_COLS; c++) {
		known_h_walls[0][c]         = true;
		known_h_walls[MAZE_ROWS][c] = true;
		h_wall_seen[0][c]           = true;
		h_wall_seen[MAZE_ROWS][c]   = true;
	}
	for (int r = 0; r < MAZE_ROWS; r++) {
		known_v_walls[r][0]         = true;
		known_v_walls[r][MAZE_COLS] = true;
		v_wall_seen[r][0]           = true;
		v_wall_seen[r][MAZE_COLS]   = true;
	}

	run_number = 0;
	SoftReset();
}

void SimpleSolver::SoftReset() {
	coord = starting_coord;
	target_coords = maze->goals;
	finished = false;
	going_back = false;
	run_number++;

	FindSurroundingWalls();
	Floodfill(run_number != 1);
}

void SimpleSolver::Step() {
	if (path.empty()) return;

	coord = path.back();
	path.pop_back();

	if (FindSurroundingWalls()) {
		if (going_back) {
			UpdateTargetCoords();
		}
		Floodfill(false);
	}

	auto it = std::find(target_coords.begin(), target_coords.end(), coord);
	if (it != target_coords.end()) {
		do {
			target_coords.erase(it);
		} while ((it = std::find(target_coords.begin(), target_coords.end(), coord)) != target_coords.end());

		if (target_coords.empty()) {
			if (going_back) {
				if (coord == starting_coord) {
					finished = true;
					target_coords = maze->goals;
				} else {
					target_coords = { starting_coord };
				}
			} else {
				going_back = true;
				UpdateTargetCoords();
			}
		}

		Floodfill(false);
	}
}

bool SimpleSolver::IsFinished() {
	return finished;
}

void SimpleSolver::DrawPath(Color clr) {
	ray::Vector2 from = maze->CellToPos(coord);
	for (int i = (int)path.size() - 1; i >= 0; i--) {
		ray::Vector2 to = maze->CellToPos(path[i]);
		DrawLineEx(from, to, 3.0f, clr);
		from = to;
	}
}

void SimpleSolver::Draw(ray::Vector2 pos, bool show_floodfill_vals, Font floodfill_font) {
	// Draw known horizontal walls
	for (int r = 0; r <= MAZE_ROWS; r++) {
		for (int c = 0; c < MAZE_COLS; c++) {
			if (!known_h_walls[r][c]) continue;
			ray::Vector2 from = pos + ray::Vector2(c * MAZE_CELL_SIZE, r * MAZE_CELL_SIZE);
			DrawLineV(from, from + ray::Vector2(MAZE_CELL_SIZE, 0.0f), BLACK);
		}
	}
	// Draw known vertical walls
	for (int r = 0; r < MAZE_ROWS; r++) {
		for (int c = 0; c <= MAZE_COLS; c++) {
			if (!known_v_walls[r][c]) continue;
			ray::Vector2 from = pos + ray::Vector2(c * MAZE_CELL_SIZE, r * MAZE_CELL_SIZE);
			DrawLineV(from, from + ray::Vector2(0.0f, MAZE_CELL_SIZE), BLACK);
		}
	}

	DrawCircleV(maze->CellToPos(coord), MAZE_CELL_SIZE * 0.4f, ORANGE);

	for (int i = 0; i < MAZE_ROWS; i++) {
		Vector2 p = maze->CornerToPos(Point(-1, i));
		GuiLabel(ray::Rectangle(p.x + 10.0f, p.y, 50.0f, 50.0f), std::to_string(i).c_str());
	}
	for (int i = 0; i < MAZE_COLS; i++) {
		Vector2 p = maze->CornerToPos(Point(i, MAZE_ROWS));
		GuiLabel(ray::Rectangle(p.x + 10.0f, p.y, 50.0f, 50.0f), std::to_string(i).c_str());
	}

	if (finished) {
		Floodfill(true);
		DrawPath(DARKBLUE);
		Floodfill(false);
		DrawPath(BLACK);
	} else {
		DrawPath(DARKBLUE);
	}

	for (Point& t : target_coords) {
		DrawCircleLinesV(maze->CellToPos(t), MAZE_CELL_SIZE * 0.4f, BLACK);
	}

	if (!show_floodfill_vals) return;

	const int TEXT_HEIGHT = 15;
	int default_text_size = GuiGetStyle(DEFAULT, TEXT_SIZE);
	int default_text_clr  = GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL);

	GuiSetFont(floodfill_font);
	GuiSetStyle(DEFAULT, TEXT_SIZE, TEXT_HEIGHT);
	GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(DARKBLUE));

	for (int r = 0; r < MAZE_ROWS; r++) {
		for (int c = 0; c < MAZE_COLS; c++) {
			if (ff_vals[r][c] == SIMPLE_FF_UNSET) continue;
			char buffer[8];
			snprintf(buffer, sizeof(buffer), "%u", ff_vals[r][c]);
			ray::Vector2 cell_pos = maze->CellToPos(Point(c, r));
			float x = cell_pos.x - GuiGetTextWidth(buffer) / 2.0f;
			float y = cell_pos.y - TEXT_HEIGHT / 2.0f;
			GuiLabel(ray::Rectangle(x, y, 50.0f, (float)TEXT_HEIGHT), buffer);
		}
	}

	GuiSetFont(GetFontDefault());
	GuiSetStyle(DEFAULT, TEXT_SIZE, default_text_size);
	GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, default_text_clr);
}
