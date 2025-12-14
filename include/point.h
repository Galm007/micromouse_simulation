#pragma once

#include <raylib-cpp.hpp>

namespace ray = raylib;

class Point {
public:
	int x, y;

	Point(int x, int y);
	Point(int a);
	~Point();

	ray::Vector2 ToVec2();

	Point operator + (Point p);
	Point operator - (Point p);
	bool operator == (Point p);
	bool operator != (Point p);
};
