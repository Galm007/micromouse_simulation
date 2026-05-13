#pragma once

#include "solver.hpp"
#include "direction.hpp"

#define FF_VAL_TO_FLOAT(n) (((n) / 3.0f) - 1.0f)
#define FF_VAL_FROM_FLOAT(x) (((x) + 1.0f) * 3.0f)

struct Edge {
	uint16_t ff_val : 10; // (0 to 1023) / 3.0f - 1.0f maps to (-1.0f to 340.0f)
	uint8_t same_dir : 7;
	uint8_t wall_exists : 1;
	Direction dir : 7;
	uint8_t visited : 1;
};

struct PathNode {
	bool horizontal;
	Point edge_coord;

	PathNode(bool horizontal, Point edge_coord) {
		this->horizontal = horizontal;
		this->edge_coord = edge_coord;
	}
};

class DiagonalSolver : public Solver {
private:
	Point coord = Point(0, 0);
	Maze* maze;

	Edge edges[2][MAZE_ROWS + 1][MAZE_COLS + 1] = { 0 }; // edges[horizontal][row][column]
	std::vector<PathNode> path;
	bool finished;
	bool going_back;
	int run_number;

	bool FindSurroundingWalls();
	void Floodfill(bool visited_edges_only);
	void UpdatePath();
	void DrawPath(Color clr);
	void UpdateTargetCoords();

public:
	std::vector<Point> target_coords;

	DiagonalSolver(Maze* maze, Point starting_coord);
	~DiagonalSolver();

	void Reset() override;
	void SoftReset() override;
	void Step() override;
	bool IsFinished() override;
	void Draw(ray::Vector2 pos, bool show_floodfill_vals, Font floodfill_font) override;
};
