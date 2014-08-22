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
#include <stdio.h>
#include <ctr/types.h>
#include <ctr/APT.h>
#include <ctr/srv.h>
#include <ctr/svc.h>
#include "svcex.h"
#include <ctr/GSP.h>
#include <ctr/GX.h>
#include <ctr/HID.h>
#include "LCD.h"
#include "draw.h"
#include "timer.h"
#include "particles.h"
#include "HIDex.h"
#include "colorslider.h"
#include <math.h>
#include <stdlib.h>

u8* topLeftFb[2];
u8* topRightFb[2];
u8* bottomFb[2];
u8 currentFb = 0;

u8* gspHeap;
u8* bottomBuf;
u32* gxCmdBuf;

Handle gspEvent,gspSharedMem;

void gspGpuInit()
{
	gspInit();

	GSPGPU_AcquireRight(NULL,0x0);
	GSPGPU_SetLcdForceBlack(NULL,0x0);

	GSPGPU_ReadHWRegs(NULL,REG_MAINLEFTFB,(u32*)topLeftFb,8);
	FB_PA_TO_VA(topLeftFb[0]);
	FB_PA_TO_VA(topLeftFb[1]);

	GSPGPU_ReadHWRegs(NULL,REG_MAINRIGHTFB,(u32*)topRightFb,8);
	FB_PA_TO_VA(topRightFb[0]);
	FB_PA_TO_VA(topRightFb[1]);

	GSPGPU_ReadHWRegs(NULL,REG_SUBFB,(u32*)bottomFb,8);
	FB_PA_TO_VA(bottomFb[0]);
	FB_PA_TO_VA(bottomFb[1]);

	u8 threadId;
	svc_createEvent(&gspEvent,RT_ONESHOT);
	GSPGPU_RegisterInterruptRelayQueue(NULL,gspEvent,0x1,&gspSharedMem,&threadId);
	svc_mapMemoryBlock(gspSharedMem,GSP_SHARED_MEM,MEMPER_READWRITE,MEMPER_DONTCARE);

	svc_controlMemory((u32*)&gspHeap,0x0,0x0,GSP_HEAP_SIZE,MEMOP_MAP_GSP_HEAP,MEMPER_READWRITE);

	svc_waitSynchronization1(gspEvent,0x55bcb0);

	gxCmdBuf = (u32*)(GSP_SHARED_MEM+0x800+threadId*0x200);

	bottomBuf = &gspHeap[MAIN_SCREEN_SIZE*3];
}

void gspGpuExit()
{
	GSPGPU_UnregisterInterruptRelayQueue(NULL);
	svc_unmapMemoryBlock(gspSharedMem,GSP_SHARED_MEM);
	svc_closeHandle(gspSharedMem);
	svc_closeHandle(gspEvent);

	gspExit();

	svc_controlMemory((u32*)&gspHeap,(u32)gspHeap,0x0,GSP_HEAP_SIZE,MEMOP_FREE,MEMPER_NONE);
}

int main(int argc,char* argv[])
{
	char buf[128];
	u64 ticks,prevticks,elapsed;
	float fps;

	ColorSlider red_slider;
	ColorSlider green_slider;
	ColorSlider blue_slider;

	initSrv();
	aptInit(APPID_APPLICATION);

	gspGpuInit();

	hidInit(NULL);

	ticks = timerInit();

	srand(svc_getSystemTick());
	init_all_particles();

	init_colorslider(&red_slider,64,255,32,CI_RED,255);
	init_colorslider(&green_slider,114,255,32,CI_GREEN,255);
	init_colorslider(&blue_slider,166,255,32,CI_BLUE,255);

	aptSetupEventHandler();

	#ifndef _SINGLETHREAD
	create_particles_thread();
	#endif

	APP_STATUS status;
	while((status=aptGetStatus())!=APP_EXITING)
	{
		switch(status)
		{
		case APP_RUNNING:
			prevticks = ticks;
			ticks = svc_getSystemTick();
			elapsed = ticksToNanoseconds(ticks - prevticks);
			fps = (float)1000000000/(float)elapsed;

			clear_screen(0,0,0,gspHeap);

			particle_properties_update();

			#ifdef _SINGLETHREAD
			update_all_particles(&pThread.request_pause);
			#endif

			//print color values
			sprintf(buf,"R:0x%02X G:0x%02X B:0x%02X",
					particleProperties.r,
					particleProperties.g,
					particleProperties.b);
			draw_string(buf,10,10,255,255,255,gspHeap);

			//print source pos
			sprintf(buf,"X:%.1f Y:%.1f",
				particleProperties.pos.x,
				particleProperties.pos.y);
			draw_string(buf,10,20,255,255,255,gspHeap);

			//print direction
			sprintf(buf,"Radians:%.4f Degrees:%.4f",
					particleProperties.dir,
					particleProperties.dir*180/3.14159f);
			draw_string(buf,10,30,255,255,255,gspHeap);

			//print size
			sprintf(buf,"Size:%d",particleProperties.size);
			draw_string(buf,10,40,255,255,255,gspHeap);

			//print gravity
			sprintf(buf,"Gravity:%s",particleProperties.gravity ? "true" : "false");
			draw_string(buf,10,50,255,255,255,gspHeap);

			clear_screen(0,0,0,bottomBuf);

			sprintf(buf,"%f FPS",fps);
			draw_string(buf,10,10,255,255,255,bottomBuf);

			update_colorslider(&red_slider);
			update_colorslider(&green_slider);
			update_colorslider(&blue_slider);

			particleProperties.r = red_slider.value;
			particleProperties.g = green_slider.value;
			particleProperties.b = blue_slider.value;

			swap();

			//toggles whether particles draw, still updates
			//only affects the multithreaded build
			/*if(REG_PAD_STATE & PAD_A)
			{
				pThread.request_pause = 1;
			}*/
			/*if(REG_PAD_STATE & PAD_START)
			{
				pThread.request_pause = 1;
			}*/

			break;

		case APP_SUSPENDING:
			aptReturnToMenu();
			break;

		case APP_SLEEPMODE:
			aptWaitStatusEvent();
			break;
		}
		timerWaitFPS();
	}

	#ifndef _SINGLETHREAD
	//tell thread to stop drawing particles
	//otherwise the thread will continue to
	//write to the gspHeap after it has been
	//freed in gspGpuExit
	pThread.request_exit = 1;
	while(1)
	{
		if(pThread.is_exitted == 1)
			break;
		else
			svc_sleepThread(1000);
	}
	#endif

	hidExit();
	gspGpuExit();
	aptExit();
	svc_exitProcess();
}
