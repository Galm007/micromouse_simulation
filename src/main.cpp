#include <iostream>
#include <raylib.h>
#include <raylib-cpp.hpp>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#include "point.h"
#include "maze.h"

namespace ray = raylib;

#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 1000
#define SCALE 4

enum ApplicationState {
	IDLE,
	PLACING_WALL,
	DELETING_WALL,
	PANNING_VIEW
};

std::string maze_filename = "maze.maz";
ApplicationState state = IDLE;
Maze maze = Maze(ray::Vector2(50.0f, 100.0f));
Point closest_corner_to_mouse = Point(0);
Point edit_wall_from = Point(0);
bool maze_is_editable = false;

Vector2 ui_anchor = { SCREEN_WIDTH - 300.0f, 0.0f };
ray::Rectangle ui_layout_recs[] = {
	ray::Rectangle(ui_anchor.x, ui_anchor.y, 300.0f, SCREEN_HEIGHT),
	ray::Rectangle(ui_anchor.x + 10.0f, ui_anchor.y + 50.0f, 280.0f, 50.0f)
};

void PreUpdate() {
	closest_corner_to_mouse = maze.ClosestCornerTo(GetMousePosition());
}

void Idle_Update() {
	if (maze_is_editable && maze.Contains(GetMousePosition())) {
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

	if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) {
		state = PANNING_VIEW;
		return;
	}
}

void PlacingWall_Update() {
	if (maze.Contains(GetMousePosition()) && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
		maze.SetWalls(edit_wall_from, closest_corner_to_mouse, true);
		state = IDLE;
		return;
	}
}

void DeletingWall_Update() {
	if (maze.Contains(GetMousePosition()) && IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
		maze.SetWalls(edit_wall_from, closest_corner_to_mouse, false);
		state = IDLE;
		return;
	}
}

void PanningView_Update() {
	if (IsMouseButtonReleased(MOUSE_BUTTON_MIDDLE)) {
		state = IDLE;
		return;
	}
	maze.position += GetMouseDelta();
}

void DrawUI() {
	GuiLabel(ray::Rectangle(20.0f, 10.0f, SCREEN_WIDTH, 50.0f), std::string("File: ").append(maze_filename).c_str());

	GuiPanel(ui_layout_recs[0], maze_filename.c_str());
	GuiToggle(ui_layout_recs[1], "EDIT MAZE", &maze_is_editable);
}

int main() {
	ray::Window window = ray::Window(SCREEN_WIDTH, SCREEN_HEIGHT, "Invaders");
	window.SetTargetFPS(60);
	GuiSetStyle(DEFAULT, TEXT_SIZE, 20);

	maze.LoadFromFile(maze_filename);

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
		case PANNING_VIEW:
			PanningView_Update();
			break;
		}

		BeginDrawing();
		window.ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

		maze.Draw(BLACK, RED);

		if (maze_is_editable) {
			Vector2 closest_corner_pos = maze.CornerToPos(closest_corner_to_mouse);
			Vector2 edit_wall_pos = maze.CornerToPos(edit_wall_from);

			if (maze.Contains(GetMousePosition())) {
				DrawCircleLinesV(closest_corner_pos, 6.0f, BLACK);
			}

			if (state == PLACING_WALL || state == DELETING_WALL) {
				if (maze.Contains(GetMousePosition())) {
					DrawLineV(
						edit_wall_pos,
						closest_corner_pos,
						Maze::IsWallValid(edit_wall_from, closest_corner_to_mouse) ? GREEN : RED
					);
				} else {
					DrawLineV(edit_wall_pos, GetMousePosition(), RED);
				}
				DrawCircleLinesV(edit_wall_pos, 6.0f, BLACK);
			}
		}

		DrawUI();

		EndDrawing();
	}

	maze.SaveToFile(maze_filename);

	return 0;
}
