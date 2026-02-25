#pragma once

#include <cstdint>
#include <raylib-cpp.hpp>

namespace ray = raylib;

class Point {
public:
	int8_t x, y;

	Point(int8_t x, int8_t y);
	Point(int8_t a);
	Point();
	~Point();

	ray::Vector2 ToVec2();

	Point operator + (Point p);
	void operator += (Point p);
	Point operator - (Point p);
	void operator -= (Point p);
	Point operator * (Point p);
	void operator *= (Point p);
	Point operator / (Point p);
	void operator /= (Point p);
	Point operator * (int8_t n);
	void operator *= (int8_t n);
	Point operator / (int8_t n);
	void operator /= (int8_t n);
	bool operator == (Point p);
	bool operator != (Point p);
};
