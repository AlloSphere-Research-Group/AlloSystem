#include "al_mainloop.h"

#include <stdio.h>
#include <stdlib.h>

void ontick(al_nsec time, void * userdata) {
	
	al_sec t = time * al_time_ns2s;
	printf("%f\n", t);
	if (t > 3.0) {
		al_main_exit();
	}
}

int main (int argc, char * argv[]) {
	
	// enter main loop
	al_main_enter(0.01, ontick, NULL, NULL);
	
	return 0;
}
