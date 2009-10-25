#include "delta.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"

int bus_proc(delta_sec t, char * args) {
	bus x = *(bus *)args;	
	/* swap buffers */
	x->front = !x->front; 
	x->data = &x->doublebuffer[SIGNAL_DIM * x->front];
	memset(x->data, 0, sizeof(sample) * SIGNAL_DIM);
	/*printf("%i ", x->front);*/
	return 0;
}

int bus_free_msg(delta_sec t, char * args) {
	bus x = *(bus *)args;
	printf("bus free %p\n", x);
	free(x);
	return 0;
}

int bus_nofree_msg(delta_sec t, char * args) {
	//bus x = *(bus *)args;
	return 0;
}

bus bus_create() {
	bus x = (bus)malloc(sizeof(struct delta_bus));
	if (x) {
	
		/* user-defined code: */
		memset(x->doublebuffer, 0, sizeof(sample) * SIGNAL_DIM * 2);
		x->front = 0;
		x->data = x->doublebuffer;
		
		/* defined above: */
		delta_audio_proc_init((process)x, bus_proc, bus_free_msg); 
	}
	return x;
}

sample * bus_read(bus self, process reader) {
	/* later readers can access the front buffer directly */
	int front = self->front ^ (reader->id < self->proc.id);
	return &self->doublebuffer[SIGNAL_DIM * front];
}