/* allocore */
#include "system/al_mainloop.h"
#include "system/al_time.h"
#include "types/al_types.h"
#include "types/al_socktube.h"

#include "io/al_AudioIO.hpp"

/* Apache Portable Runtime */
#include "apr_general.h"
#include "apr_errno.h"
#include "apr_pools.h"
#include "apr_thread_proc.h"
#include "apr_thread_mutex.h"

#include "stdlib.h"

#define MAX_MESSAGE_LEN (AL_SOCKTUBE_MAXPACKETSIZE+1)

int active = 1;
al_socktube sock = NULL;
int parentcount = 0;
int childcount = 0;

/* APR threads/mutexes require a memory pool */
static apr_pool_t * main_mempool;

static apr_status_t check_apr(apr_status_t err) {
	char errstr[1024];
	if (err != APR_SUCCESS) {
		apr_strerror(err, errstr, 1024);
		printf("apr error: %s\n", errstr);
	}
	return err;
}

void audioCB(allo::AudioIOData& io){
	char imsg[MAX_MESSAGE_LEN];
	char omsg[MAX_MESSAGE_LEN];
	
	printf("~");
	memset(imsg, 0, MAX_MESSAGE_LEN);		
	int readbytes = al_socktube_child_read(sock, imsg);
	while (readbytes > 0) {
		//printf("child read %d %d '%s'\n", readbytes, strlen(imsg), imsg);
		
		memset(imsg, 0, MAX_MESSAGE_LEN);
		readbytes = al_socktube_child_read(sock, imsg);
	}
	
	// write a ping
	memset(omsg, 0, MAX_MESSAGE_LEN);
	sprintf(omsg, "audio %d", childcount++);
	if (al_socktube_child_write(sock, omsg, strlen(omsg))) {
		printf("child write error\n");
	}
	
	
//	size_t len = 0;
//	do {
//		memset(imsg, 0, MAX_MESSAGE_LEN);
//		len = al_socktube_child_read(sock, imsg);
//		while (len) {
//			//printf("child read %d %d '%s'\n", len, strlen(imsg), imsg);
//			
//			memset(omsg, 0, MAX_MESSAGE_LEN);
//			sprintf(omsg, "reply %d ", childcount++);
//			if (al_socktube_child_write(sock, omsg, strlen(omsg))) {
//				printf("child write error\n");
//			}
//			
//			memset(imsg, 0, MAX_MESSAGE_LEN);
//			len = al_socktube_child_read(sock, imsg);
//		}
//	} while(len);
}

void ontick(al_nsec time, void * userdata) {
	
	al_sec t = time * al_time_ns2s;
	
	printf(".");

	char imsg[MAX_MESSAGE_LEN];
	memset(imsg, 0, MAX_MESSAGE_LEN);
	size_t len = al_socktube_parent_read(sock, (void *)imsg);
	while (len) {
		printf("parent read %d '%s'\n", len, imsg);
		len = al_socktube_parent_read(sock, imsg);
	}
	
	for (int i=0; i<1000; i++) {
		char omsg[MAX_MESSAGE_LEN];
		memset(omsg, 0, MAX_MESSAGE_LEN);
		sprintf(omsg, "sent %d @%f ", parentcount++, t);
		if (al_socktube_parent_write(sock, omsg, strlen(omsg))) {
			printf("parent write error\n");
		}
	}
		
	if (t > 9.0) {
		active = 0;
		al_main_exit();
	}
}

void onquit(void * userdata) {}

int main (int argc, char * argv[]) {
	
	// init APR:
	check_apr(apr_initialize());
	atexit(apr_terminate);	// ensure apr tear-down
	
	// create mempool:
	check_apr(apr_pool_initialize());
	check_apr(apr_pool_create(&main_mempool, NULL));
	apr_allocator_max_free_set(apr_pool_allocator_get(main_mempool), 1024);

	sock = al_socktube_create(); 
	
	allo::AudioIO audioIO(64, 44100, audioCB, NULL, 2,2);
	audioIO.start();

	// enter main loop
	al_main_enter(0.01, ontick, NULL, onquit);

	audioIO.stop();
	
	al_socktube_free(&sock);
	
	// free main memory pool:
	apr_pool_destroy(main_mempool);
	
	return 0;
}