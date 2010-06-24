
#include <stdio.h>
#include <stdlib.h>

#include "types/al_MsgQueue.hpp"

using namespace al;

void test_msg(al_sec t, char * data) {
	printf("@%f Msg %s\n", t, data);
}

static char * prefix = "prefix";

int main (int argc, char * argv[]) {
	
	MsgQueue q;
	
	q.sched(1, test_msg, prefix, strlen(prefix));
	q.sched(2, test_msg, prefix, strlen(prefix));
	q.sched(3, test_msg, prefix, strlen(prefix));
	
	q.update(4);

	return 0;
}
