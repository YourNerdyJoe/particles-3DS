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
#include "particles.h"
#include "LCD.h"
#include <ctr/svc.h>
#include "HIDex.h"
#include <math.h>
#include <stdlib.h>
#include "draw.h"

Particle particles[MAX_PARTICLES];
ParticleProperties particleProperties = { 
	{80.0f,120.0f},
	0,
	1,
	255,255,255,
	1,
};

Vector gravityForce = {0.0f,-0.08f};

extern u8* gspHeap;

void init_all_particles()
{
	int i;
	for(i = 0; i < MAX_PARTICLES; i++)
	{
		restart_particle(&particles[i]);
	}
}

void update_particle(Particle* p,u8 paused)
{
	if(p->lifetime > 0)
	{
		//reduce lifetime
		p->lifetime--;

		//apply gravity
		if(particleProperties.gravity)
			vec_add(&p->speed,&p->speed,&gravityForce);

		//move particle
		vec_add(&p->pos,&p->pos,&p->speed);
	}
	else if(!paused)
	{
		//restart when lifetime == 0
		restart_particle(p);
	}
}

void draw_particle(Particle* p)
{
	if(p->lifetime == 0) return;

	//draw particle
	int x,y,size;
	x = (int)p->pos.x;
	y = (int)p->pos.y;
	size = particleProperties.size;

	y = MAIN_SCREEN_HEIGHT-y;

	if(x<0 || y<0 || x+size>=MAIN_SCREEN_WIDTH || y+size>=MAIN_SCREEN_HEIGHT) return;

	draw_fillrect(	x, y ,x+size-1, y+size-1,
					particleProperties.r,
					particleProperties.g,
					particleProperties.b,
					gspHeap);
}

void restart_particle(Particle* p)
{
	p->lifetime = (rand() % 100) + 1;

	float x,y;
	x = particleProperties.pos.x + rand()%16;
	y = particleProperties.pos.y + rand()%16;

	vec_set(&p->pos,x,y);

	float ct,st;
	float theta = particleProperties.dir + (float)(rand() % 100)/100 - 0.5f;
	ct = cosf(theta)*4;
	st = sinf(theta)*4;

	vec_set(&p->speed,ct,st);
}

void update_all_particles(vu8* paused)
{
	int i;
	for(i = 0; i < MAX_PARTICLES; i++)
	{
		update_particle(&particles[i],*paused);
		draw_particle(&particles[i]);
	}
}

/*static void update_all_particles_maybe_draw(vu8* dontrestart)
{
	int i;
	for(i = 0; i < MAX_PARTICLES; i++)
	{
		update_particle(&particles[i]);
		draw_particle(&particles[i]);
	}
}*/

void draw_all_particles()
{
	int i;
	for(i = 0; i < MAX_PARTICLES; i++)
	{
		draw_particle(&particles[i]);
	}
}

u32 ptStack[THREAD_STACKSIZE];
vThread pThread;

void init_thread(vThread* t)
{
	t->thread = 0;
	t->request_pause = 0;
	t->is_paused = 0;
	t->request_exit = 0;
	t->is_exitted = 0;
}

Result create_particles_thread()
{
	init_thread(&pThread);

	return svc_createThread(
		(Handle*)&pThread.thread,
		particles_thread,
		(u32)&pThread,
		&ptStack[THREAD_STACKSIZE],
		0x3f,
		~1
	);
}

void particles_thread(u32 info)
{
	vThread* pt = (vThread*)info;

	while(1)
	{
		if(pt->request_exit)
		{
			pt->is_exitted = 1;
			svc_exitThread();
			return;
		}
		else
		{
			if(pt->request_pause)
			{
				pt->is_paused = 1;
			}
			else
			{
				pt->is_paused = 0;
			}
			update_all_particles(&pt->request_pause);

			svc_sleepThread(1000);
		}
	}
}

static void particle_properties_update_move()
{
	//move particle source
	u32 pad = REG_PAD_STATE;
	if(pad & PAD_LEFT) {
		particleProperties.pos.x -= PARTICLE_SOURCE_SPEED;
	}
	if(pad & PAD_RIGHT) {
		particleProperties.pos.x += PARTICLE_SOURCE_SPEED;
	}
	if(pad & PAD_DOWN) {
		particleProperties.pos.y -= PARTICLE_SOURCE_SPEED;
	}
	if(pad & PAD_UP) {
		particleProperties.pos.y += PARTICLE_SOURCE_SPEED;
	}
}

static void particle_properties_update_direction()
{
	vCirclePadInfo* cpad = REG_CPAD_INFO;
	
	float distance = sqrt((float)(cpad->x*cpad->x + cpad->y*cpad->y));
	if(distance >= 48)
	{
		float direction = atan2f((float)cpad->y/distance,(float)cpad->x/distance);

		particleProperties.dir = direction;
	}
}

static u32 prevpad = 0;

static void particle_properties_update_size()
{
	u32 pad = REG_PAD_STATE;

	if((pad & PAD_A) && !(prevpad & PAD_A))
	{
		particleProperties.size++;
	}
	if((pad & PAD_B) && !(prevpad & PAD_B))
	{
		particleProperties.size--;
	}

	if(particleProperties.size < 1) particleProperties.size = 1;
	else if(particleProperties.size > 64) particleProperties.size = 64;
}

static void particle_properties_update_gravity()
{
	u32 pad = REG_PAD_STATE;

	if((pad & PAD_X) && !(prevpad & PAD_X))
	{
		particleProperties.gravity ^= 1;
	}
}

static void particle_properties_update_toggle_paused()
{
	u32 pad = REG_PAD_STATE;

	if((pad & PAD_START) && !(prevpad & PAD_START))
	{
		pThread.request_pause ^= 1;
	}
}

void particle_properties_update()
{
	particle_properties_update_move();
	particle_properties_update_direction();
	particle_properties_update_size();
	particle_properties_update_gravity();
	particle_properties_update_toggle_paused();
	prevpad = REG_PAD_STATE;
}
