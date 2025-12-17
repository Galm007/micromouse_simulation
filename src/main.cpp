#include <iostream>
#include <raylib.h>
#include <raylib-cpp.hpp>

#define RAYGUI_IMPLEMENTATION
#define GUI_WINDOW_FILE_DIALOG_IMPLEMENTATION
#include <gui_window_file_dialog.h>

#include "point.h"
#include "maze.h"
#include "solver.h"
#include "mouse.h"
#include "console.h"

namespace ray = raylib;

#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 1000

enum ApplicationState {
	IDLE,
	PLACING_WALL,
	DELETING_WALL,
	MOVING_STARTING_COORD,
	SAVING_MAZE,
	LOADING_MAZE,
	SOLVING_MAZE
};

// Miscellaneous variables
ApplicationState state = IDLE;
std::string maze_filename;
Point closest_corner_to_mouse = Point(0);
Point edit_wall_from = Point(0);
bool maze_is_editable = false;
bool show_manhattan_dist = false;
bool show_full_map = true;
float solver_step_interval = 0.90f;
float step_timer = 1.0f;
bool panning_view = false;
bool showing_paths = false;
std::vector<std::vector<Point>> paths;

// Core entities
Maze maze = Maze(ray::Vector2(100.0f, 100.0f));
Solver solver = Solver(&maze, Point(0, 0));
Mouse mouse = Mouse(&maze, Point(0, 0));

// UI layout
ray::Vector2 ui_anchor = ray::Vector2(SCREEN_WIDTH - 300.0f, 0.0f);
ray::Vector2 console_anchor = ray::Vector2(0.0f, SCREEN_HEIGHT - 150.0f);
ray::Rectangle ui_layout_recs[] = {
	ray::Rectangle(ui_anchor.x, ui_anchor.y, 300.0f, SCREEN_HEIGHT), // Panel
	ray::Rectangle(ui_anchor.x + 10.0f, ui_anchor.y + 50.0f, 280.0f, 50.0f), // Load Maze Layout Button
	ray::Rectangle(ui_anchor.x + 10.0f, ui_anchor.y + 110.0f, 280.0f, 50.0f), // Save Maze Layout Button
	ray::Rectangle(ui_anchor.x + 10.0f, ui_anchor.y + 930.0f, 280.0f, 50.0f), // Edit Maze Toggle
	ray::Rectangle(ui_anchor.x + 10.0f, ui_anchor.y + 870.0f, 280.0f, 50.0f), // Clear Maze
	ray::Rectangle(ui_anchor.x + 10.0f, ui_anchor.y + 190.0f, 280.0f, 50.0f), // Run Solver
	ray::Rectangle(ui_anchor.x + 10.0f, ui_anchor.y + 490.0f, 280.0f, 50.0f), // Stop Solver
	ray::Rectangle(ui_anchor.x + 10.0f, ui_anchor.y + 260.0f, 30.0f, 30.0f), // Manhattan Distance
	ray::Rectangle(ui_anchor.x + 10.0f, ui_anchor.y + 320.0f, 30.0f, 30.0f), // Full Map
	ray::Rectangle(ui_anchor.x + 70.0f, ui_anchor.y + 380.0f, 160.0f, 30.0f), // Solver Speed Slider
	ray::Rectangle(ui_anchor.x + 10.0f, ui_anchor.y + 430.0f, 280.0f, 50.0f), // Skip Animation
	ray::Rectangle(console_anchor.x, console_anchor.y, 900.0f, 150.0f), // Console
	ray::Rectangle(console_anchor.x + 820.0f, console_anchor.y + 2.0f, 70.0f, 20.0f) // Clear Console
};
GuiWindowFileDialogState file_dialog_state;

void PreUpdate() {
	closest_corner_to_mouse = maze.ClosestCornerTo(GetMousePosition());
}

void PostUpdate() {
	// Pan the view
	if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
		maze.position += GetMouseDelta();
		mouse.position += GetMouseDelta();
	}

	mouse.Move(40 * GetFrameTime());
	mouse.angle += GetFrameTime();
}

void Idle_Update() {
	ray::Vector2 m = GetMousePosition();

	if (m.x < ui_anchor.x) {
		Point mouse_coord = Point(
			(m.x - maze.position.x) / MAZE_CELL_SIZE,
			(m.y - maze.position.y) / MAZE_CELL_SIZE
		);

		if (maze_is_editable) {
			// Controls for adding/deleting Walls
			if (maze.Contains(m)) {
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
		} else if (mouse_coord == solver.starting_coord && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			// Control for dragging the starting coord somewhere else
			state = MOVING_STARTING_COORD;
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

void MovingStartingCoord_Update() {
	ray::Vector2 m = GetMousePosition();
	if (m.x < ui_anchor.x && maze.Contains(m) && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
		solver.starting_coord = Point(
			(m.x - maze.position.x) / MAZE_CELL_SIZE,
			(m.y - maze.position.y) / MAZE_CELL_SIZE
		);
		state = IDLE;
	}
}

void FileDialogLogic() {
	// When a file is selected from the built-in file browser
	if (file_dialog_state.SelectFilePressed) {
		if (IsFileExtension(file_dialog_state.fileNameText, ".maz")) {
			std::string filename = std::string(file_dialog_state.dirPathText)
				.append("/")
				.append(file_dialog_state.fileNameText);

			if (state == SAVING_MAZE) {
				// Save maze layout and starting position to file
				maze.SaveToFile(filename, solver.starting_coord);
				maze_filename = filename;
				SetWindowTitle(file_dialog_state.fileNameText);
			} else if (state == LOADING_MAZE) {
				// Load maze layout and starting position from file
				if (ray::FileExists(filename)) {
					maze.LoadFromFile(filename, &solver.starting_coord);
					maze_filename = filename;
					SetWindowTitle(file_dialog_state.fileNameText);
				} else {
					ConsoleError("Unable to open file: " + filename);
				}
			}
		} else {
			ConsoleError("Invalid filetype!");
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

void SolvingMaze_Update() {
	if ((step_timer -= GetFrameTime()) <= 0.0f && !showing_paths) {
		step_timer = 1.0f - solver_step_interval;

		// Calculate the next step and move
		if (solver.Step()) {
			if (solver.target_coords[0] != solver.starting_coord) {
				// When the goal is reached, set the destination back to the starting coord
				solver.target_coords = { solver.starting_coord };
				step_timer = 1.0f;
			} else {
				paths = solver.FindSolutions();
				showing_paths = true;
			}
		}
	}
}

void DrawUI() {
	// Display the name of the maze file currently loaded
	GuiLabel(
		ray::Rectangle(20.0f, 10.0f, SCREEN_WIDTH, 50.0f),
		std::string("File: ").append(maze_filename).c_str()
	);

	// UI controls on the right side
	GuiPanel(ui_layout_recs[0], "Micromouse Simulator");
	if (GuiButton(ui_layout_recs[1], "LOAD MAZE LAYOUT")) {
		file_dialog_state.windowActive = true;
		state = LOADING_MAZE;
	}
	if (GuiButton(ui_layout_recs[2], "SAVE MAZE LAYOUT")) {
		file_dialog_state.windowActive = true;
		state = SAVING_MAZE;
	}
	if (state != SOLVING_MAZE) {
		GuiToggle(ui_layout_recs[3], "EDIT MAZE", &maze_is_editable);
		if (maze_is_editable && GuiButton(ui_layout_recs[4], "CLEAR WALLS")) {
			maze.Clear();
		}
	}
	if (!maze_is_editable && GuiButton(ui_layout_recs[5], "RUN SOLVER")) {
		solver.Reset();
		maze_is_editable = false;
		step_timer = 0.5f;
		showing_paths = false;
		state = SOLVING_MAZE;
	}
	if (state == SOLVING_MAZE) {
		if (GuiButton(ui_layout_recs[6], "STOP SOLVER")) {
			state = IDLE;
		}
		GuiCheckBox(ui_layout_recs[7], "Manhattan Distance", &show_manhattan_dist);
		GuiCheckBox(ui_layout_recs[8], "Full Map", &show_full_map);
		GuiSlider(ui_layout_recs[9], "SLOW", "FAST", &solver_step_interval, 0.1f, 1.0f);
		if (GuiButton(ui_layout_recs[10], "SKIP ANIMATION")) {
			while (!showing_paths) {
				step_timer = 0.0f;
				SolvingMaze_Update();
			}
		}
	}
	ConsoleDraw(ui_layout_recs[11]);
	if (GuiButton(ui_layout_recs[12], "clear")) {
		ConsoleClear();
	}

	// Display file dialogue for saving/loading files
	GuiWindowFileDialog(&file_dialog_state);
}

int main(int argc, char** argv) {
	ray::Window window = ray::Window(SCREEN_WIDTH, SCREEN_HEIGHT, "Micromouse");
	window.SetTargetFPS(60);
	GuiSetStyle(DEFAULT, TEXT_SIZE, 20);

	// Maze file name can be put in as a command line argument
	if (argc == 1) {
		ConsoleLog("Restoring previous session...");
		maze_filename = "backup.maz";
		maze.LoadFromFile(maze_filename, &solver.starting_coord);
		SetWindowTitle(maze_filename.c_str());
	} else if (argc == 2) {
		maze_filename = argv[1];
		maze.LoadFromFile(maze_filename, &solver.starting_coord);
		SetWindowTitle(maze_filename.c_str());
	} else {
		std::cout << "Too many input arguments!" << std::endl;
		return 1;
	}

	// Initialize file dialogue state
	file_dialog_state = InitGuiWindowFileDialog(GetWorkingDirectory());
	file_dialog_state.windowBounds = ray::Rectangle(100.0f, 250.0f, 1000.0f, 500.0f);
	file_dialog_state.saveFileMode = true;

	while (!window.ShouldClose()) {
		// Update logic
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
		case MOVING_STARTING_COORD:
			MovingStartingCoord_Update();
			break;
		case SAVING_MAZE:
		case LOADING_MAZE:
			FileDialogLogic();
			break;
		case SOLVING_MAZE:
			SolvingMaze_Update();
			break;
		}
		PostUpdate();

		// ------ DRAWING ------ //

		BeginDrawing();
		window.ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

		// Color the target area green
		DrawRectangleV(
			maze.CornerToPos(Point(7, 7)),
			ray::Vector2(2.0f, 2.0f) * MAZE_CELL_SIZE,
			ColorAlpha(GREEN, 0.5f)
		);

		// Draw the maze
		Color wall_clr = BLACK;
		if (state == SOLVING_MAZE) {
			wall_clr = show_full_map ? LIGHTGRAY : ColorAlpha(BLACK, 0.0f);
		}
		maze.Draw(wall_clr, RED);

		if (maze_is_editable) {
			Vector2 closest_corner_pos = maze.CornerToPos(closest_corner_to_mouse);
			Vector2 edit_wall_pos = maze.CornerToPos(edit_wall_from);

			// Show which corner is affected by the mouse while editing
			if (maze.Contains(GetMousePosition())) {
				DrawCircleLinesV(closest_corner_pos, 6.0f, BLACK);
			}

			// Draw the walls that are about to be edited by the user in green.
			// If the action is invalid, draw the line in red.
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
		} else if (state != SOLVING_MAZE) {
			// Draw the solver's starting coord
			DrawCircleV(
				state == MOVING_STARTING_COORD
					? GetMousePosition()
					: maze.CellToPos(solver.starting_coord),
				MAZE_CELL_SIZE * 0.4f,
				ORANGE
			);
		}

		// Draw the solver on top of the maze
		if (state == SOLVING_MAZE) {
			solver.Draw(maze.position, show_manhattan_dist);

			// Draw the solutions
			if (showing_paths) {
				for (int p = 0; p < paths.size(); p++) {
					Point from = paths[p][0];
					for (int i = 1; i < paths[p].size(); i++) {
						DrawLineEx(
							maze.CellToPos(from),
							maze.CellToPos(paths[p][i]),
							2.0f,
							PURPLE
						);
						from = paths[p][i];
					}
				}
			}
		}

		mouse.Draw();
		DrawUI();
		EndDrawing();
	}

	maze.SaveToFile("backup.maz", solver.starting_coord);
	return 0;
}
