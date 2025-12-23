#pragma once

#include "point.h"

enum Direction {
	DIR_UNKNOWN,

	// Possible moves from horizontal cell edges
	DIR_UP,
	DIR_UP_LEFT,
	DIR_UP_RIGHT,
	DIR_DOWN,
	DIR_DOWN_LEFT,
	DIR_DOWN_RIGHT,

	// Possible moves from vertical cell edges
	DIR_LEFT,
	DIR_LEFT_UP,
	DIR_LEFT_DOWN,
	DIR_RIGHT,
	DIR_RIGHT_UP,
	DIR_RIGHT_DOWN
};

Direction NormalizeDir(Direction dir);
Direction ReverseDir(Direction dir);
Direction ComplementDir(Direction dir);
void GetNextPossibleEdges(bool dest_horizontals[3], Point dest_edges[3], Direction normalized_dir);
bool SimilarDirections(Direction d1, Direction d2);
Point DirToCell(Point coord, Direction dir);
std::string DirToStr(Direction dir);
