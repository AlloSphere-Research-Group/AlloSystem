#include "types/al_socktube.h"

#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#ifndef AF_LOCAL
#define AF_LOCAL AF_UNIX
#endif

/*
	Reader needs to know how much to read.
	Could prepend each packet with a header to indicate size.
	A single char can signify up to 255 bytes; is it big enough for you?
		this is enough for 31 doubles... or 31 pointers on a 64-bit machine
		or a string of 254 characters...
		anything bigger than that should be malloc'd anyway, right?
*/

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

int al_socktube_parent_read(al_socktube x, void * buffer) {
	/* read one byte: */
	char size;
	if (read(x->socks[0], &size, 1) <= 0) {
		return 0;
	}
	//ssize_t r = read(x->socks[0], buffer, size);
	ssize_t r = recv(x->socks[0], buffer, size, MSG_DONTWAIT);
	if (r != size) {
		printf("parent read %d %d\n", (int)r, size);
		perror("parent read bad data");
		return 0;
	}
	return size;
}

int al_socktube_child_read(al_socktube x, void * buffer) {
	/* read one byte: */
	char size;
	if (read(x->socks[1], &size, 1) <= 0) {
		return 0;
	}
	ssize_t r = read(x->socks[1], buffer, size);
	if (r != size) {
		printf("child read %d %d\n", (int)r, size);
		perror("child read bad data");
		return 0;
	}
	return size;
}

int al_socktube_parent_write(al_socktube x, void * buffer, size_t size) {
	char data[AL_SOCKTUBE_MAXPACKETSIZE+1];
	data[0] = size;
	memcpy(data+1, buffer, size);
	size++;
	// TODO: isn't this essentially blocking?
	ssize_t r;
	//do {
		//r = write(x->socks[0], data, size);
		r = send(x->socks[0], data, size, MSG_DONTWAIT);
	//} while (r == -1);
	if (r != (ssize_t)size) {
		printf("parent write %d %d\n", (int)r, (int)size);
		perror("failed to write data");
		return -1;
	}
	return 0;
}

int al_socktube_child_write(al_socktube x, void * buffer, size_t size) {
	char data[AL_SOCKTUBE_MAXPACKETSIZE+1];
	data[0] = size;
	memcpy(data+1, buffer, size);
	size++;
	// TODO: isn't this essentially blocking?
	ssize_t r;
	//do {
		r = write(x->socks[0], data, size);
	//} while (r == -1);
	if (r != (ssize_t)size) {
		printf("child write %d %d\n", (int)r, (int)size);
		perror("failed to write data");
		return -1;
	}
	return 0;
}