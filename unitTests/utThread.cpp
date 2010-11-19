#include "utAllocore.h"

//int active = 1;
//
//void * threadfunc1(void * ud) {
//	int i = 10;
//	while (i-- && active) {
//		al_sleep(0.1);
//		printf("tf1 %d\n", i);
//	}
//	return 0;
//}

void * threadFunc(void * user){
	*(int *)user = 1; return NULL;
}

int utThread() {
//	Thread thd(threadfunc1, 0);
//	al_sleep(0.2);
//	active = 0;
//	thd.wait();
//	printf("thread done\n");

	// Thread
	UT_PRINTF("system: thread\n");
	{	
		int x=0;
		Thread t(threadFunc, &x);
		t.wait();
		assert(x == 1);
	}

	return 0;
}
