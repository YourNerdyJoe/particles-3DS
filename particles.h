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
#ifndef _PARTICLES_H_
#define _PARTICLES_H_
#include <ctr/types.h>

typedef struct Vector {
	float x,y;
} Vector;

typedef struct Particle{
	Vector pos;
	Vector speed;
	u32 lifetime;
} Particle;

typedef struct ParticleProperties
{
	Vector pos;
	float dir;
	int size;
	u8 r,g,b;
	u8 gravity;
} ParticleProperties;

typedef struct Thread {
	Handle thread;
	u8 request_pause;
	u8 is_paused;
	u8 request_exit;
	u8 is_exitted;
} Thread;

typedef volatile Thread vThread;

#define MAX_PARTICLES 4096
extern Particle particles[MAX_PARTICLES];
extern ParticleProperties particleProperties;

#define PARTICLE_SOURCE_SPEED	4

static inline void vec_set(Vector* dst,float x,float y) {
	dst->x = x;
	dst->y = y;
}

static inline void vec_add(Vector* dst,Vector* src1,Vector* src2) {
	dst->x = src1->x + src2->x;
	dst->y = src1->y + src2->y;
}

#define THREAD_STACKSIZE 0x100

extern u32 ptStack[THREAD_STACKSIZE];	//1kB
extern vThread pThread;

void init_all_particles();
void update_particle(Particle* p,u8 paused);
void draw_particle(Particle* p);
void restart_particle(Particle* p);
void update_all_particles(vu8* paused);
void draw_all_particles();
void init_thread(vThread* t);
Result create_particles_thread();
void particles_thread(u32 info);
void particle_properties_update();

#endif
