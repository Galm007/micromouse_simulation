#pragma once

#include "solver.hpp"

#define SIMPLE_FF_UNSET UINT16_MAX

class SimpleSolver : public Solver {
private:
	Point coord;
	Maze* maze;

	bool known_h_walls[MAZE_ROWS + 1][MAZE_COLS] = {};
	bool known_v_walls[MAZE_ROWS][MAZE_COLS + 1] = {};
	bool h_wall_seen[MAZE_ROWS + 1][MAZE_COLS] = {};
	bool v_wall_seen[MAZE_ROWS][MAZE_COLS + 1] = {};
	bool cell_visited[MAZE_ROWS][MAZE_COLS] = {};
	uint16_t ff_vals[MAZE_ROWS][MAZE_COLS] = {};

	std::vector<Point> path;
	bool finished = false;
	bool going_back = false;
	int run_number = 0;

	bool FindSurroundingWalls();
	void Floodfill(bool visited_cells_only);
	void UpdatePath();
	void UpdateTargetCoords();
	void DrawPath(Color clr);

public:
	std::vector<Point> target_coords;

	SimpleSolver(Maze* maze, Point starting_coord);
	~SimpleSolver();

	void Reset() override;
	void SoftReset() override;
	void Step() override;
	bool IsFinished() override;
	void Draw(ray::Vector2 pos, bool show_floodfill_vals, Font floodfill_font) override;
};
