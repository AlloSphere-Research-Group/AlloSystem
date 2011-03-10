#include "utAllocore.h"

void * threadFunc(void * user){
	*(int *)user = 1; return NULL;
}

struct MyThreadFunc : public ThreadFunction{
	MyThreadFunc(int& x_): x(x_){}
	void operator()(){
		x = 1;
	}
	int& x;
};

int utThread() {

	//UT_PRINTF("system: thread\n");

	// POSIX-style
	{	
		int x=0;
		Thread t(threadFunc, &x);
		t.join();
		assert(1 == x);
	}

	// C++ style
	{
		int x = 0;
		MyThreadFunc f(x);
		Thread t(f);
		t.join();
		assert(1 == x);
	}

	return 0;
}
