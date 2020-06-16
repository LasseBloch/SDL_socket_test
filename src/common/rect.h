//
// Created by Lasse Bloch Lauritsen on 14/06/2020.
//

#ifndef SDL_SOCKETTEST_RECT_H
#define SDL_SOCKETTEST_RECT_H

#include <cstdint>

class Rect {
public:
	Rect(int16_t x, int16_t y, int16_t width, int16_t height)
	{
		this->x = x;
		this->y = y;
		this->width = width;
		this->height = height;
	}

	int16_t x;
	int16_t y;
	int16_t width;
	int16_t height;
};

#endif //SDL_SOCKETTEST_RECT_H
