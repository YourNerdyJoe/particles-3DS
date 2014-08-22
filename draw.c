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
#include "draw.h"
#include <string.h>
#include <ctr/types.h>
#include <ctr/GSP.h>
#include <ctr/GX.h>
#include "LCD.h"
#include "svcex.h"
#include "ascii64.h"

extern u8* topLeftFb[2];
extern u8* topRightFb[2];
extern u8* bottomFb[2];
extern u8 currentFb;

extern u8* gspHeap;
extern u8* bottomBuf;
extern u32* gxCmdBuf;

void copyBuffer()
{
	u8 copiedFb = currentFb^1;
	
	GSPGPU_FlushDataCache(NULL,gspHeap,MAIN_SCREEN_SIZE*3*2);

	GX_RequestDma(gxCmdBuf,(u32*)gspHeap,(u32*)topLeftFb[copiedFb],MAIN_SCREEN_SIZE*3);

	GX_RequestDma(gxCmdBuf,(u32*)gspHeap,(u32*)topRightFb[copiedFb],MAIN_SCREEN_SIZE*3);

	GX_RequestDma(gxCmdBuf,(u32*)bottomBuf,(u32*)bottomFb[copiedFb],SUB_SCREEN_SIZE*3);
}

void swapBuffer()
{
	u32 regData;
	GSPGPU_ReadHWRegs(NULL,REG_MAINFB_SELECT,&regData,4);
	regData ^= 1;
	currentFb = regData&1;
	GSPGPU_WriteHWRegs(NULL,REG_MAINFB_SELECT,&regData,4);
	GSPGPU_WriteHWRegs(NULL,REG_SUBFB_SELECT,&regData,4);
}

void swap()
{
	copyBuffer();
	swapBuffer();
}

void draw_pixel(int x, int y, char r, char g, char b, u8* screen)
{
    int coord = 720*x+720-(y*3)-3;
    screen[coord+0] = b;
	screen[coord+1] = g;
	screen[coord+2] = r;
}

void draw_char(char letter,int x,int y, char r, char g, char b, u8* screen)
{
  int i, k;
  unsigned char mask;
  unsigned char l;

  for (i = 0; i < 8; i++){
    mask = 0b10000000;
    l = ascii_data[letter][i];
    for (k = 0; k < 8; k++){
      if ((mask >> k) & l){
        draw_pixel(k+x,i+y,r,g,b,screen);
      }     
    }
  }
}

void draw_string(char* word, int x,int y, char r, char g, char b, u8* screen)
{
	int tmp_x =x;
	int i;
	int line = 0;

	int width;

	if(screen == bottomBuf)
		width = SUB_SCREEN_WIDTH;
	else
		width = MAIN_SCREEN_WIDTH;

	size_t len = strlen(word);
	for (i = 0; i < len; i++)
	{
		if (tmp_x+8 > width) {
			line++;
			tmp_x = x;
		}
		draw_char(word[i],tmp_x,y+(line*8),r,g,b, screen);

		tmp_x = tmp_x+8;
	}
}

void draw_fillrect(int x1, int y1, int x2, int y2, char r, char g, char b, u8* screen)
{
	int i,j;
	for(i=x1;i<=x2;i++){
		for(j=y1;j<=y2;j++){
			draw_pixel(i,j, r, g, b, screen);
		}
	}
}

void clear_screen(char r, char g, char b, u8* screen)
{
	int width;
	if(screen == bottomBuf)
		width = SUB_SCREEN_WIDTH;
	else
		width = MAIN_SCREEN_WIDTH;

	u32 x,y;
	for(x = 0; x < width; x++)
	{
		for(y = 0; y < 240; y++)
		{
			u32 scr_pix = (x*240+y)*3;
			screen[scr_pix+0] = b;
			screen[scr_pix+1] = g;
			screen[scr_pix+2] = r;
		}
	}
}
