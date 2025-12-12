#include <iostream>
#include <raylib.h>
#include <raylib-cpp.hpp>

#include "point.h"
#include "maze.h"

namespace ray = raylib;

#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 1000
#define SCALE 4

enum ApplicationState {
	IDLE,
	PLACING_WALL,
	DELETING_WALL
};

ApplicationState state = IDLE;
Maze maze = Maze(ray::Vector2(50.0f, 50.0f));
Point closest_corner_to_mouse = Point(0);
Point edit_wall_from = Point(0);

void PreUpdate() {
	closest_corner_to_mouse = maze.ClosestCornerTo(GetMousePosition());
}

void Idle_Update() {
	if (maze.Contains(GetMousePosition())) {
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			edit_wall_from = closest_corner_to_mouse;
			state = PLACING_WALL;
			return;
		}
		if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
			edit_wall_from = closest_corner_to_mouse;
			state = DELETING_WALL;
			return;
		}
	}
}

void PlacingWall_Update() {
	Vector2 maze_pos = maze.GetPosition();
	if (maze.Contains(GetMousePosition()) && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
		maze.SetWalls(edit_wall_from, closest_corner_to_mouse, true);
		state = IDLE;
		return;
	}
}

void DeletingWall_Update() {
	Vector2 maze_pos = maze.GetPosition();
	if (maze.Contains(GetMousePosition()) && IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
		maze.SetWalls(edit_wall_from, closest_corner_to_mouse, false);
		state = IDLE;
		return;
	}
}

int main() {
	ray::Window window = ray::Window(SCREEN_WIDTH, SCREEN_HEIGHT, "Invaders");
	window.SetTargetFPS(60);

	maze.LoadFromFile("maze.maze");

	while (!window.ShouldClose()) {
		float ft = GetFrameTime();

		PreUpdate();
		switch (state) {
		case IDLE:
			Idle_Update();
			break;
		case PLACING_WALL:
			PlacingWall_Update();
			break;
		case DELETING_WALL:
			DeletingWall_Update();
			break;
		}

		BeginDrawing();
		window.ClearBackground(WHITE);

		maze.Draw(BLACK, RED);

		Vector2 closest_corner_pos = maze.CornerToPos(closest_corner_to_mouse);
		if (maze.Contains(GetMousePosition())) {
			DrawCircleLinesV(closest_corner_pos, 6.0f, BLACK);
		}

		Vector2 edit_wall_pos = maze.CornerToPos(edit_wall_from);
		if (maze.Contains(GetMousePosition()) && (state == PLACING_WALL || state == DELETING_WALL)) {
			DrawLineV(
				edit_wall_pos,
				closest_corner_pos,
				Maze::IsWallValid(edit_wall_from, closest_corner_to_mouse) ? GREEN : RED
			);
			DrawCircleLinesV(edit_wall_pos, 6.0f, BLACK);
		}

		EndDrawing();
	}

	maze.SaveToFile("maze.maze");

	return 0;
}
