#include "al_main.h"
#include "stdio.h"

void tick(al_ns time, void * userdata) {
	
	double t = al_ns2s(time);
	printf("%f\n", al_now());
	if (t > 3.0) {
		al_main_exit();
	}
}

int main (int argc, char * const argv[]) {
    
	printf("enter main loop\n");
	al_main_enter(0.01, tick, NULL);
	
	
	printf("done\n");
	al_quit();
	
	return 0;
}
