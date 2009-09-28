#include "al_main.h"
#include "stdlib.h"

/*
	Platform specific implementations
		(implemented in separate file)
*/
extern void al_main_platform_attach(double interval);
extern int al_main_platform_enter(double interval);

/*
	If there is anything that is a singleton, it's the main loop!
*/	
static al_main_t * g_main;

#pragma mark C API

al_main_t * al_init() {
	if (g_main == 0) {
		g_main = (al_main_t *)malloc(sizeof(al_main_t));
		g_main->t0 = al_time_cpu();
	}
	return g_main;
}

void al_quit() {
	if (g_main != 0) {
		free(g_main);
		g_main = 0;
	}
}

int al_main_enter(double interval, main_tick_handler handler, void * userdata) {
	al_init();	
	if (!g_main->isRunning) {
		g_main->interval = interval;
		g_main->isRunning = 1;
		g_main->handler = handler;
		g_main->userdata = userdata;
		return al_main_platform_enter(interval);
	}
	return 0;
}

void al_main_exit() {
	if (g_main->isRunning) {
		g_main->isRunning = 0;
	}
}

void al_main_attach(double interval, main_tick_handler handler, void * userdata) {
	if (!g_main->isRunning) {
		g_main->interval = interval;
		g_main->isRunning = 1;
		g_main->handler = handler;
		g_main->userdata = userdata;
		al_main_platform_attach(interval);
	}
}

void al_main_tick() {
	g_main->logicaltime = al_time_cpu() - g_main->t0;
	// pass control to user-specified mainloop:
	(g_main->handler)(g_main->logicaltime, g_main->userdata);
}

al_ns al_time() {
	return g_main->logicaltime;
}