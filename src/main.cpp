#include <iostream>
#include <raylib.h>
#include <raylib-cpp.hpp>

#define RAYGUI_IMPLEMENTATION
#define GUI_WINDOW_FILE_DIALOG_IMPLEMENTATION
#include <gui_window_file_dialog.h>

#include "point.h"
#include "maze.h"
#include "mouse.h"

namespace ray = raylib;

#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 1000

enum ApplicationState {
	IDLE,
	PLACING_WALL,
	DELETING_WALL,
	PANNING_VIEW,
	SAVING_MAZE,
	LOADING_MAZE
};

ApplicationState state = IDLE;

std::string maze_filename;
Maze maze = Maze(ray::Vector2(50.0f, 100.0f));
Point closest_corner_to_mouse = Point(0);
Point edit_wall_from = Point(0);
bool maze_is_editable = false;

Mouse mouse = Mouse(&maze, Point(0, 0));

GuiWindowFileDialogState file_dialog_state;
Vector2 ui_anchor = { SCREEN_WIDTH - 300.0f, 0.0f };
ray::Rectangle ui_layout_recs[] = {
	ray::Rectangle(ui_anchor.x, ui_anchor.y, 300.0f, SCREEN_HEIGHT), // Panel
	ray::Rectangle(ui_anchor.x + 10.0f, ui_anchor.y + 50.0f, 280.0f, 50.0f), // Load Maze Layout Button
	ray::Rectangle(ui_anchor.x + 10.0f, ui_anchor.y + 110.0f, 280.0f, 50.0f), // Save Maze Layout Button
	ray::Rectangle(ui_anchor.x + 10.0f, ui_anchor.y + 930.0f, 280.0f, 50.0f), // Edit Maze Toggle
	ray::Rectangle(ui_anchor.x + 10.0f, ui_anchor.y + 870.0f, 280.0f, 50.0f), // Clear Maze
};

void PreUpdate() {
	closest_corner_to_mouse = maze.ClosestCornerTo(GetMousePosition());
}

void Idle_Update() {
	Vector2 mouse = GetMousePosition();

	if (mouse.x < ui_anchor.x) {
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
	mouse.position += GetMouseDelta();
}

void FileDialogLogic() {
	if (file_dialog_state.SelectFilePressed) {
		if (IsFileExtension(file_dialog_state.fileNameText, ".maz")) {
			std::string filename = std::string(file_dialog_state.dirPathText)
				.append("/")
				.append(file_dialog_state.fileNameText);

			if (state == SAVING_MAZE) {
				maze.SaveToFile(filename);
				maze_filename = filename;
			} else if (state == LOADING_MAZE) {
				if (ray::FileExists(filename)) {
					maze.LoadFromFile(filename);
					maze_filename = filename;
				} else {
					std::cout << "Unable to open file: " << filename << std::endl;
				}
			}
		} else {
			std::cout << "Invalid filetype!" << std::endl;
		}

		file_dialog_state.SelectFilePressed = false;
		state = IDLE;
		return;
	}

	if (!file_dialog_state.windowActive) {
		state = IDLE;
		return;
	}
}

void DrawUI() {
	GuiLabel(ray::Rectangle(20.0f, 10.0f, SCREEN_WIDTH, 50.0f), std::string("File: ").append(maze_filename).c_str());

	GuiPanel(ui_layout_recs[0], "Micromouse Simulator");
	if (GuiButton(ui_layout_recs[1], "LOAD MAZE LAYOUT")) {
		file_dialog_state.windowActive = true;
		state = LOADING_MAZE;
	}
	if (GuiButton(ui_layout_recs[2], "SAVE MAZE LAYOUT")) {
		file_dialog_state.windowActive = true;
		state = SAVING_MAZE;
	}
	GuiToggle(ui_layout_recs[3], "EDIT MAZE", &maze_is_editable);
	if (GuiButton(ui_layout_recs[4], "CLEAR WALLS")) {
		maze.Clear();
	}

	GuiWindowFileDialog(&file_dialog_state);
}

int main(int argc, char** argv) {
	ray::Window window = ray::Window(SCREEN_WIDTH, SCREEN_HEIGHT, "Micromouse");
	window.SetTargetFPS(60);
	GuiSetStyle(DEFAULT, TEXT_SIZE, 20);

	if (argc == 1) {
		std::cout << "Restoring previous session..." << std::endl;
		maze_filename = "backup.maz";
		maze.LoadFromFile(maze_filename);
	} else if (argc == 2) {
		maze_filename = argv[1];
		maze.LoadFromFile(maze_filename);
	} else {
		std::cout << "Too many input arguments!" << std::endl;
		return 1;
	}

	file_dialog_state = InitGuiWindowFileDialog(GetWorkingDirectory());
	file_dialog_state.windowBounds = ray::Rectangle(100.0f, 250.0f, 1000.0f, 500.0f);
	file_dialog_state.saveFileMode = true;

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
		case SAVING_MAZE:
		case LOADING_MAZE:
			FileDialogLogic();
			break;
		}

		mouse.Move(40 * GetFrameTime());
		mouse.angle += GetFrameTime();

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

		mouse.Draw();

		DrawUI();

		EndDrawing();
	}

	maze.SaveToFile("backup.maz");

	return 0;
}
