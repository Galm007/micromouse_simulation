#include "maze.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <raylib.h>
#include <raylib-cpp.hpp>

namespace ray = raylib;

Maze::Maze(Vector2 position) {
	this->position = position;

	for (int row = 0; row < MAZE_ROWS; row++) {
		for (int col = 0; col < MAZE_COLS; col++) {
			horizontal_walls[row][col] = false;
			vertical_walls[row][col] = false;
		}

		vertical_walls[row][0] = true;
		vertical_walls[row][MAZE_COLS] = true;
	}

	for (int col = 0; col < MAZE_COLS; col++) {
		horizontal_walls[0][col] = true;
		horizontal_walls[MAZE_ROWS][col] = true;
	}
}

Maze::~Maze() {

}

bool Maze::Contains(Vector2 pos) {
	return !(pos.x < position.x
		|| pos.x > position.x + MAZE_COLS * MAZE_CELL_SIZE
		|| pos.y < position.y
		|| pos.y > position.y + MAZE_ROWS * MAZE_CELL_SIZE);
}

Point Maze::ClosestCornerTo(Vector2 pos) {
	if (!Contains(pos)) {
		return Point(-1);
	}

	return Point(
		(pos.x - position.x + MAZE_CELL_SIZE / 2.0f) / MAZE_CELL_SIZE,
		(pos.y - position.y + MAZE_CELL_SIZE / 2.0f) / MAZE_CELL_SIZE
	);
}

Vector2 Maze::CornerToPos(Point coord) {
	return ray::Vector2(
		position.x + coord.x * MAZE_CELL_SIZE,
		position.y + coord.y * MAZE_CELL_SIZE
	);
}

bool Maze::IsWallValid(Point from_corner, Point to_corner) {
	return (from_corner.y == to_corner.y && from_corner.x != to_corner.x && to_corner.y != 0 && to_corner.y != MAZE_ROWS)
		|| (from_corner.y != to_corner.y && from_corner.x == to_corner.x && to_corner.x != 0 && to_corner.x != MAZE_COLS);
}

void Maze::SetWalls(Point from_corner, Point to_corner, bool state) {
	if (from_corner.y == to_corner.y) {
		int left = std::min(from_corner.x, to_corner.x);
		int right = std::max(from_corner.x, to_corner.x);

		for (int col = left; col < right; col++) {
			horizontal_walls[to_corner.y][col] = state;
		}
	} else if (from_corner.x == to_corner.x) {
		int top = std::min(from_corner.y, to_corner.y);
		int down = std::max(from_corner.y, to_corner.y);

		for (int row = top; row < down; row++) {
			vertical_walls[row][to_corner.x] = state;
		}
	}
}

void Maze::Draw(Color wall_clr, Color dot_clr) {
	for (int row = 0; row < MAZE_ROWS; row++) {
		for (int col = 0; col < MAZE_COLS; col++) {
			bool left_wall = vertical_walls[row][col];
			bool top_wall = horizontal_walls[row][col];

			int x = col * MAZE_CELL_SIZE + position.x;
			int y = row * MAZE_CELL_SIZE + position.y;

			if (left_wall) {
				DrawLine(x, y, x, y + MAZE_CELL_SIZE, wall_clr);
			}

			if (top_wall) {
				DrawLine(x, y, x + MAZE_CELL_SIZE, y, wall_clr);
			}

			DrawCircle(x, y, 3.0f, dot_clr);
		}
	}

	int right = position.x + MAZE_COLS * MAZE_CELL_SIZE;
	int bottom = position.y + MAZE_ROWS * MAZE_CELL_SIZE;

	DrawLine(right, position.y, right, bottom, wall_clr);
	DrawLine(position.x, bottom, right, bottom, wall_clr);

	for (int row = 0; row <= MAZE_ROWS; row++) {
		DrawCircle(right, row * MAZE_CELL_SIZE + position.y, 3.0f, dot_clr);
	}
	for (int col = 0; col < MAZE_COLS; col++) {
		DrawCircle(col * MAZE_CELL_SIZE + position.x, bottom, 3.0f, dot_clr);
	}
}

int Maze::SaveToFile(std::string filename) {
	std::ofstream file;
	file.open(filename);
	if (!file.is_open()) {
		std::cout << "Unable to open file: " << filename << std::endl;
		return 0;
	}

	for (int row = 0; row < MAZE_ROWS; row++) {
		std::string line;
		for (int col = 0; col < MAZE_COLS; col++) {
			bool left_wall = vertical_walls[row][col];
			bool top_wall = horizontal_walls[row][col];

			char c = '0';
			if (left_wall && !top_wall) {
				c = '1';
			} else if (!left_wall && top_wall) {
				c = '2';
			} else if (left_wall && top_wall) {
				c = '3';
			}
			line.push_back(c);
		}

		file << line << '\n';
	}

	std::cout << "Saved maze layout to: " << filename << std::endl;

	file.close();
	return 1;
}

int Maze::LoadFromFile(std::string filename) {
	std::ifstream file;
	file.open(filename);
	if (!file.is_open()) {
		std::cout << "Unable to open file: " << filename << std::endl;
		return 0;
	}

	// Each cell is represented by a number:
	// 0 - no left or upper walls
	// 1 - wall on the left
	// 2 - wall on the top
	// 3 - wall on both left and top
	
	// Maze edges will automatically have walls

	std::string line;
	int row = 0;
	while (std::getline(file, line)) {
		int col = 0;
		for (char& c : line) {
			switch (c) {
				case '0':
					horizontal_walls[row][col] = false;
					vertical_walls[row][col] = false;
					break;
				case '1':
					horizontal_walls[row][col] = false;
					vertical_walls[row][col] = true;
					break;
				case '2':
					horizontal_walls[row][col] = true;
					vertical_walls[row][col] = false;
					break;
				case '3':
					horizontal_walls[row][col] = true;
					vertical_walls[row][col] = true;
					break;
				case '\n':
					break;
				default:
					std::cout << "Invalid cell state: " << c << std::endl;
					return 0;
			}
			col++;
		}
		row++;
	}

	std::cout << "Loaded maze layout: " << filename << std::endl;

	file.close();
	return 1;
}
