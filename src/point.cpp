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
