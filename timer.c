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
#include "timer.h"
#include <ctr/types.h>
#include <ctr/svc.h>

static u64 timer_ticks,timer_tpf;
static u64 tickstons = 1000000000/268123480;

u64 timerInit()
{
	timer_ticks = svc_getSystemTick();
	timer_tpf = 1000000000/50;

	return timer_ticks;
}

void timerWaitFPS()
{
	u64 elapsed = ticksToNanoseconds(svc_getSystemTick() - timer_ticks);
	if(elapsed < timer_tpf)
	{
		svc_sleepThread(timer_tpf - elapsed);
	}
	timer_ticks = svc_getSystemTick();
}

u64 ticksToNanoseconds(u64 ticks)
{
	return ticks*tickstons;
}
