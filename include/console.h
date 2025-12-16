#pragma once

#include <string>
#include <raylib.h>
#include <raylib-cpp.hpp>

namespace ray = raylib;

void ConsoleLog(std::string msg);
void ConsoleDraw(ray::Rectangle rec);
void ConsoleClear();
