#include "al_mainloop.h"
#include "math/al_Complex.hpp"

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

	al::Complex<double> c(0.3, 0.1);
	al::Complex<double> c2 = c.pow(0.5);
	
	// enter main loop
//	al_main_enter(0.01, ontick, NULL, NULL);
	
	return 0;
}
