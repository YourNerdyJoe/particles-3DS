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
#include "colorslider.h"
#include "LCD.h"
#include "HIDex.h"

extern u8* bottomBuf;

void init_colorslider(ColorSlider* cs,int y,int w,int h,u8 color_index,u8 init_value)
{
	cs->x = SUB_SCREEN_WIDTH/2 - w/2;
	cs->y = y;
	cs->w = w;
	cs->h = h;
	cs->color_index = color_index;
	cs->value = init_value;
}

static void draw_colorslider(ColorSlider* cs)
{
	int x,y,i;
	for(x = 0; x < cs->w; x++)
	{
		int color = 255*x/cs->w;
		for(y = 0; y < cs->h; y++)
		{
			i = ((cs->x+x)*SUB_SCREEN_HEIGHT+SUB_SCREEN_HEIGHT-(cs->y+y+1))*3;
			bottomBuf[i + cs->color_index] = (u8)color;
		}
	}

	x = cs->x + (cs->value*cs->w/255);
	i = ((x+1)*SUB_SCREEN_HEIGHT)*3;

	for(y = cs->y-4; y < cs->y+cs->h+4; y++)
	{
		int v = i - (y+1)*3;
		bottomBuf[v+0] = 255;
		bottomBuf[v+1] = 255;
		bottomBuf[v+2] = 255;
	}
}

void update_colorslider(ColorSlider* cs)
{
	vTouchScreenInfo* touch = REG_TOUCH_INFO_PIXEL;

	if(touch->contains_data && touch->y >= cs->y && touch->y < cs->y+cs->h)
	{
		int value = 255*(touch->x - cs->x)/cs->w;
		if(value < 0) value = 0;
		else if(value > 255) value = 255;

		cs->value = value;
	}

	draw_colorslider(cs);
}
