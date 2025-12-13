#include "point.h"

Point::Point(int x, int y) {
	this->x = x;
	this->y = y;
}

Point::Point(int a) {
	this->x = this->y = a;
}

Point::~Point() {

}

ray::Vector2 Point::ToVec2() {
	return ray::Vector2(x, y);
}
