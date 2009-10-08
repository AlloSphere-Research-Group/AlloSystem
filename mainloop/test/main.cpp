#include "al_main.h"
#include "stdio.h"

void tick(al_nsec time, void * userdata) {
	
	al_sec t = al_nsec2sec(time);
	printf("%f\n", al_now());
	if (t > 3.0) {
		al_main_exit();
	}
}

void glutTick() {
	al_main_tick();
}

int main (int argc, char * const argv[]) {
    
	printf("enter main loop\n");
	al_main_enter(0.01, tick, NULL);
	
	// OSX:
	//al_main_attach(0.01, tick, NUUL);
	
	printf("done\n");
	al_quit();
	
	return 0;
}
