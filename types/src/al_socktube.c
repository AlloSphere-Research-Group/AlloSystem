#include "al_socktube.h"

#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#ifndef AF_LOCAL
#define AF_LOCAL AF_UNIX
#endif

al_socktube al_socktube_create() {
	al_socktube x = (al_socktube)malloc(sizeof(al_socktube_t));
	if (x) {
		/* create socket pair */
		if (socketpair(AF_LOCAL, SOCK_STREAM, 0, x->socks) < 0) {
			perror("opening stream socket pair");
			free(x);
			x = NULL;
		} else {
			/* set non-blocking */
			fcntl(x->socks[0], F_SETFL, O_NONBLOCK);
			fcntl(x->socks[1], F_SETFL, O_NONBLOCK);
		}
	}
	return x;
}

void al_socktube_free(al_socktube * ptr) {
	if (ptr && *ptr) {
		al_socktube x = *ptr;
		close(x->socks[0]);
		close(x->socks[1]);
		free(x);
		*ptr = NULL;
	}
}

int al_socktube_parent_read(al_socktube x, char * buffer, size_t len) {
	int res = read(x->socks[0], buffer, len);
	//printf("res %d len %d\n", res, len);
	return res > 0 ? res : 0;
}

int al_socktube_child_read(al_socktube x, char * buffer, size_t len) {
	int res = read(x->socks[1], buffer, len);
	//printf("res %d len %d\n", res, len);
	return res > 0 ? res : 0;
}

int al_socktube_parent_write(al_socktube x, char * buffer, size_t len) {
	return write(x->socks[0], buffer, len) != len;
}

int al_socktube_child_write(al_socktube x, char * buffer, size_t len) {
	return write(x->socks[1], buffer, len) != len;
}