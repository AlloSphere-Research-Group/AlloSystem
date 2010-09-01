#include "utAllocore.h"
#include "system/al_Thread.hpp"

int active = 1;

void * threadfunc1(void * ud) {
	int i = 10;
	while (i-- && active) {
		al_sleep(0.1);
		printf("tf1 %d\n", i);
	}
	return 0;
}


int utThread() {
	Thread thd(threadfunc1, 0);
	al_sleep(0.2);
	active = 0;
	thd.wait();
	printf("thread done\n");
	
	return 0;
}
