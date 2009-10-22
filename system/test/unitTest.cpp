#include <assert.h>
#include "al_thread.h"
#include "al_time.h"

using namespace allo;

THREAD_FUNCTION(threadFunc){
	*(int *)user = 1; return NULL;
}

int main(int argc, char* argv[]){

	// Thread
	{	int x=0;
		Thread t(threadFunc, &x);
		t.wait();
		assert(x == 1);
	}
	
	al_sleep_sec(1);

	return 0;
}
