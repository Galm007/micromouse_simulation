#include "direction.h"
#include "console.h"

Direction NormalizeDir(Direction dir) {
	switch (dir) {
	case DIR_UP:
	case DIR_LEFT_UP:
	case DIR_RIGHT_UP:
		return DIR_UP;
	case DIR_DOWN:
	case DIR_LEFT_DOWN:
	case DIR_RIGHT_DOWN:
		return DIR_DOWN;
	case DIR_LEFT:
	case DIR_UP_LEFT:
	case DIR_DOWN_LEFT:
		return DIR_LEFT;
	case DIR_RIGHT:
	case DIR_UP_RIGHT:
	case DIR_DOWN_RIGHT:
		return DIR_RIGHT;
	default:
		ConsoleError("Direction is Invalid!");
		return DIR_UNKNOWN;
	}
}

Direction ReverseDir(Direction dir) {
	switch (dir) {
	case DIR_UP:
		return DIR_DOWN;
	case DIR_LEFT_UP:
		return DIR_DOWN_RIGHT;
	case DIR_RIGHT_UP:
		return DIR_DOWN_LEFT;
	case DIR_DOWN:
		return DIR_UP;
	case DIR_LEFT_DOWN:
		return DIR_UP_RIGHT;
	case DIR_RIGHT_DOWN:
		return DIR_UP_LEFT;
	case DIR_LEFT:
		return DIR_RIGHT;
	case DIR_UP_LEFT:
		return DIR_RIGHT_DOWN;
	case DIR_DOWN_LEFT:
		return DIR_RIGHT_UP;
	case DIR_RIGHT:
		return DIR_LEFT;
	case DIR_UP_RIGHT:
		return DIR_LEFT_DOWN;
	case DIR_DOWN_RIGHT:
		return DIR_LEFT_UP;
	default:
		ConsoleError("Direction is Invalid!");
		return DIR_UNKNOWN;
	}
}

Direction ComplementDir(Direction dir) {
	switch (dir) {
	case DIR_UP:
		return DIR_UP;
	case DIR_LEFT_UP:
		return DIR_UP_LEFT;
	case DIR_RIGHT_UP:
		return DIR_UP_RIGHT;
	case DIR_DOWN:
		return DIR_DOWN;
	case DIR_LEFT_DOWN:
		return DIR_DOWN_LEFT;
	case DIR_RIGHT_DOWN:
		return DIR_DOWN_RIGHT;
	case DIR_LEFT:
		return DIR_LEFT;
	case DIR_UP_LEFT:
		return DIR_LEFT_UP;
	case DIR_DOWN_LEFT:
		return DIR_LEFT_DOWN;
	case DIR_RIGHT :
		return DIR_RIGHT;
	case DIR_UP_RIGHT:
		return DIR_RIGHT_UP;
	case DIR_DOWN_RIGHT:
		return DIR_RIGHT_DOWN;
	default:
		ConsoleError("Direction is Invalid!");
		return DIR_UNKNOWN;
	}
}

void GetNextPossibleEdges(bool dest_horizontals[3], Point dest_edges[3], Direction normalized_dir) {
	switch (normalized_dir) {
	case DIR_UP:
		dest_horizontals[0] = true;
		dest_edges[0] = Point(0, -1); // DIR_UP

		dest_horizontals[1] = false;
		dest_edges[1] = Point(0, -1); // DIR_UP_LEFT

		dest_horizontals[2] = false;
		dest_edges[2] = Point(1, -1); // DIR_UP_RIGHT
		break;
	case DIR_DOWN:
		dest_horizontals[0] = true;
		dest_edges[0] = Point(0, 1); // DIR_DOWN

		dest_horizontals[1] = false;
		dest_edges[1] = Point(0, 0); // DIR_DOWN_LEFT

		dest_horizontals[2] = false;
		dest_edges[2] = Point(1, 0); // DIR_DOWN_RIGHT
		break;
	case DIR_LEFT:
		dest_horizontals[0] = false;
		dest_edges[0] = Point(-1, 0); // DIR_LEFT

		dest_horizontals[1] = true;
		dest_edges[1] = Point(-1, 0); // DIR_LEFT_UP

		dest_horizontals[2] = true;
		dest_edges[2] = Point(-1, 1); // DIR_LEFT_DOWN
		break;
	case DIR_RIGHT:
		dest_horizontals[0] = false;
		dest_edges[0] = Point(1, 0); // DIR_RIGHT

		dest_horizontals[1] = true;
		dest_edges[1] = Point(0, 0); // DIR_RIGHT_UP

		dest_horizontals[2] = true;
		dest_edges[2] = Point(0, 1); // DIR_RIGHT_DOWN
		break;
	default:
		ConsoleError("Direction is not normalized!");
		break;
	}
}

bool SimilarDirections(Direction d1, Direction d2) {
	return (d1 == DIR_UP && d2 == DIR_UP)
		|| (d1 == DIR_DOWN && d2 == DIR_DOWN)
		|| (d1 == DIR_LEFT && d2 == DIR_LEFT)
		|| (d1 == DIR_RIGHT && d2 == DIR_RIGHT)
		|| (d1 == DIR_UP_LEFT && d2 == DIR_LEFT_UP) || (d2 == DIR_UP_LEFT && d1 == DIR_LEFT_UP)
		|| (d1 == DIR_UP_RIGHT && d2 == DIR_RIGHT_UP) || (d2 == DIR_UP_RIGHT && d1 == DIR_RIGHT_UP)
		|| (d1 == DIR_DOWN_LEFT && d2 == DIR_LEFT_DOWN) || (d2 == DIR_DOWN_LEFT && d1 == DIR_LEFT_DOWN)
		|| (d1 == DIR_DOWN_RIGHT && d2 == DIR_RIGHT_DOWN) || (d2 == DIR_DOWN_RIGHT && d1 == DIR_RIGHT_DOWN);
}

std::string DirToStr(Direction dir) {
	switch (dir) {
	case DIR_UP:
		return "U";
	case DIR_LEFT_UP:
		return "LU";
	case DIR_RIGHT_UP:
		return "RU";
	case DIR_DOWN:
		return "D";
	case DIR_LEFT_DOWN:
		return "LD";
	case DIR_RIGHT_DOWN:
		return "RD";
	case DIR_LEFT:
		return "L";
	case DIR_UP_LEFT:
		return "UL";
	case DIR_DOWN_LEFT:
		return "DL";
	case DIR_RIGHT:
		return "R";
	case DIR_UP_RIGHT:
		return "UR";
	case DIR_DOWN_RIGHT:
		return "DR";
	default:
		ConsoleError("Direction is Invalid!");
		return "_";
	}
}
