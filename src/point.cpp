#include "point.h"

Point::Point(int8_t x, int8_t y) {
	this->x = x;
	this->y = y;
}

Point::Point(int8_t a) {
	this->x = this->y = a;
}

Point::Point() { }

Point::~Point() { }

ray::Vector2 Point::ToVec2() {
	return ray::Vector2(x, y);
}

Point Point::operator + (Point p) {
	return Point(x + p.x, y + p.y);
}

void Point::operator += (Point p) {
	x += p.x;
	y += p.y;
}

Point Point::operator - (Point p) {
	return Point(x - p.x, y - p.y);
}

void Point::operator -= (Point p) {
	x -= p.x;
	y -= p.y;
}

Point Point::operator * (Point p) {
	return Point(x * p.x, y * p.y);
}

void Point::operator *= (Point p) {
	x *= p.x;
	y *= p.y;
}

Point Point::operator / (Point p) {
	return Point(x / p.x, y / p.y);
}

void Point::operator /= (Point p) {
	x /= p.x;
	y /= p.y;
}

Point Point::operator * (int8_t n) {
	return Point(x * n, y * n);
}

void Point::operator *= (int8_t n) {
	x *= n;
	y *= n;
}

Point Point::operator / (int8_t n) {
	return Point(x / n, y / n);
}

void Point::operator /= (int8_t n) {
	x /= n;
	y /= n;
}

bool Point::operator == (Point p) {
	return x == p.x && y == p.y;
}

bool Point::operator != (Point p) {
	return x != p.x || y != p.y;
}
