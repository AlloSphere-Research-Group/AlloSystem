#include "al_mainloop.h"

#include <stdio.h>
#include <stdlib.h>

void tick(al_nsec time, void * userdata) {
	
	al_sec t = time * al_time_ns2s;
	printf("%f\n", al_main_time_sec());
	if (t > 3.0) {
		al_main_exit();
	}
}

int main (int argc, char * argv[]) {
	
	// enter main loop
	al_main_enter(0.01, tick, NULL);
	
	al_main_exit();
	return 0;
}
