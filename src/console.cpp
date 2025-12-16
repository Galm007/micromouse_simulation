#include <algorithm>
#include <iostream>
#include <raylib.h>
#include <raygui.h>
#include "console.h"

#define LINE_HEIGHT 25

std::vector<std::string> logs;
ray::Vector2 scroll = ray::Vector2(0.0f, 0.0f);
ray::Rectangle content_rec = ray::Rectangle(0.0f, 0.0f, 0.0f, 10.0f);

void ConsoleLog(std::string msg) {
	logs.push_back(msg);
	std::cout << msg << std::endl;

	// Adjust content_rec size to accomodate the message
	content_rec.width = std::max(content_rec.width, (float)GuiGetTextWidth(msg.c_str())) + 10.0f;
	content_rec.height += LINE_HEIGHT;
}

void ConsoleDraw(ray::Rectangle rec) {
	ray::Rectangle scissor;
	GuiScrollPanel(rec, "Console", content_rec, &scroll, &scissor);

	BeginScissorMode(scissor.x, scissor.y, scissor.width, scissor.height);
	for (int i = 0; i < logs.size(); i++) {
		ray::Rectangle r = ray::Rectangle(
			rec.x + 5.0f + scroll.x,
			rec.y + scroll.y + LINE_HEIGHT * (i + 1),
			content_rec.width,
			LINE_HEIGHT
		);
		if (i > 0) {
			r.y += 10.0f; // separate the most recent log from the others
		}
		GuiLabel(r, logs[logs.size() - 1 - i].c_str());
	}
	EndScissorMode();
}

void ConsoleClear() {
	logs.clear();
	content_rec.width = 0.0f;
	content_rec.height = 10.0f;
}
