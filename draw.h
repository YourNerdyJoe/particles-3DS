/*
	3DS Particle Demo
	Copyright (C) 2014 Joseph LoManto

	This software is provided 'as-is', without any express or implied 
	warranty. In no event will the authors be held liable for any damages 
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

	1. The origin of this software must not be misrepresented; you must not 
	   claim that you wrote the original software. If you use this software 
	   in a product, an acknowledgment in the product documentation would be 
	   appreciated but is not required.
	2. Altered source versions must be plainly marked as such, and must not be 
	   misrepresented as being the original software.
	3. This notice may not be removed or altered from any source distribution.
*/
#ifndef _DRAW_H_
#define _DRAW_H_
#include <ctr/types.h>

void swap();

void draw_pixel(int x, int y, char r, char g, char b, u8* screen);
void draw_char(char letter,int x,int y, char r, char g, char b, u8* screen);
void draw_string(char* word, int x,int y, char r, char g, char b, u8* screen);
void draw_fillrect( int x1, int y1, int x2, int y2, char r, char g, char b, u8* screen);
void clear_screen(char r, char g, char b, u8* screen);

#endif
