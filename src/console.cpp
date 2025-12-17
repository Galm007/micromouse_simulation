#include <algorithm>
#include <iostream>
#include <raylib.h>
#include <raygui.h>

#include "console.h"

#define LINE_HEIGHT 25

std::vector<std::string> logs;
ray::Vector2 scroll = ray::Vector2(0.0f, 0.0f);
ray::Rectangle content_rec = ray::Rectangle(0.0f, 0.0f, 0.0f, 10.0f);
bool recent_error = false;

void ConsoleLog(std::string msg) {
	logs.push_back(msg);
	std::cout << msg << std::endl;
	recent_error = false;

	// Adjust content_rec size to accomodate the message
	content_rec.width = std::max(content_rec.width, (float)GuiGetTextWidth(msg.c_str())) + 10.0f;
	content_rec.height += LINE_HEIGHT;
}

void ConsoleError(std::string msg) {
	ConsoleLog(msg);
	recent_error = true;
}

void ConsoleDraw(ray::Rectangle rec) {
	ray::Rectangle scissor;
	GuiScrollPanel(rec, "Console", content_rec, &scroll, &scissor);

	int default_text_clr = GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL);

	BeginScissorMode(scissor.x, scissor.y, scissor.width, scissor.height);
	for (int i = 0; i < logs.size(); i++) {
		const char* msg = logs[logs.size() - 1 - i].c_str();
		ray::Rectangle r = ray::Rectangle(
			rec.x + 5.0f + scroll.x,
			rec.y + scroll.y + LINE_HEIGHT * (i + 1),
			content_rec.width,
			LINE_HEIGHT
		);
		if (i == 0) {
			GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(recent_error ? RED : BLUE));
			GuiLabel(r, msg);
			GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, default_text_clr);
		} else {
			r.y += 10.0f; // separate the most recent log from the others
			GuiLabel(r, msg);
		}
	}
	EndScissorMode();
}

void ConsoleClear() {
	logs.clear();
	content_rec.width = 0.0f;
	content_rec.height = 10.0f;
}
