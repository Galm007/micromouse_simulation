#include "mouse.h"
#include "raylib.h"

Mouse::Mouse(Maze* maze, Point starting_coord) {
	position = maze->position + (starting_coord.ToVec2() + 0.5f) * MAZE_CELL_SIZE;
}

Mouse::~Mouse() {
}

ray::Vector2 Mouse::Forward() {
	return ray::Vector2(cos(angle), sin(angle));
}

ray::Vector2 Mouse::Right() {
	return ray::Vector2(cos(angle + PI / 2.0f), sin(angle + PI / 2.0f));
}

void Mouse::Move(float amount) {
	position += Forward() * amount;
}

void Mouse::Draw() {
	// Front
	DrawLineEx(position, position + Forward() * MAZE_CELL_SIZE / 2.0f, 3.0f, GREEN);
	DrawLineEx(position, position + Right() * MAZE_CELL_SIZE / 2.0f, 3.0f, GREEN);
	DrawLineEx(position, position - Right() * MAZE_CELL_SIZE / 2.0f, 3.0f, GREEN);

	// Body
	ray::Rectangle rect = ray::Rectangle(position.x, position.y, 20.0f, 20.0f);
	DrawRectanglePro(rect, ray::Vector2(10.0f, 10.0f), angle * RAD2DEG, BLUE);
}
