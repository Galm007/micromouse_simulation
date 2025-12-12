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
	PLACING_WALL
};

ApplicationState state = IDLE;
Maze maze = Maze(ray::Vector2(50.0f, 50.0f));
Point closest_corner_to_mouse = Point(0);
Point place_wall_from = Point(0);

void PreUpdate() {
	closest_corner_to_mouse = maze.ClosestCornerTo(GetMousePosition());
}

void Idle_Update() {
	if (maze.Contains(GetMousePosition()) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
		place_wall_from = closest_corner_to_mouse;
		state = PLACING_WALL;
		return;
	}
}

void PlacingWall_Update() {
	Vector2 maze_pos = maze.GetPosition();
	if (maze.Contains(GetMousePosition()) && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
		maze.AddWall(place_wall_from, closest_corner_to_mouse);
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
		}

		BeginDrawing();
		window.ClearBackground(WHITE);

		maze.Draw(BLACK, RED);

		Vector2 closest_corner_pos = maze.CornerToPos(closest_corner_to_mouse);
		if (maze.Contains(GetMousePosition())) {
			DrawCircleLinesV(closest_corner_pos, 6.0f, BLACK);
		}

		Vector2 place_wall_pos = maze.CornerToPos(place_wall_from);
		if (maze.Contains(GetMousePosition()) && state == PLACING_WALL) {
			DrawLineV(
				place_wall_pos,
				closest_corner_pos,
				Maze::IsWallValid(place_wall_from, closest_corner_to_mouse) ? GREEN : RED
			);
			DrawCircleLinesV(place_wall_pos, 6.0f, BLACK);
		}

		EndDrawing();
	}

	maze.SaveToFile("maze.maze");

	return 0;
}
