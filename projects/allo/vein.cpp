/* allocore */
#include "system/al_mainloop.h"
#include "system/al_time.h"
#include "types/al_types.h"
#include "types/al_socktube.h"

/* Apache Portable Runtime */
#include "apr_general.h"
#include "apr_errno.h"
#include "apr_pools.h"
#include "apr_thread_proc.h"
#include "apr_thread_mutex.h"

#include "stdlib.h"

#define MAX_MESSAGE_LEN (4096)

int active = 1;
al_socktube sock = NULL;
int parentcount = 0;
int childcount = 0;

/* APR threads/mutexes require a memory pool */
static apr_pool_t * main_mempool;
static apr_thread_mutex_t * sub_mutex;
static apr_threadattr_t * sub_thread_attr;
static apr_thread_t * sub_thread;

static apr_status_t check_apr(apr_status_t err) {
	char errstr[1024];
	if (err != APR_SUCCESS) {
		apr_strerror(err, errstr, 1024);
		printf("apr error: %s\n", errstr);
	}
	return err;
}


void * subthreadfunc(apr_thread_t * thread, void * userdata) {
		
	while(active) {
		al_sleep(0.001);
//		check_apr(apr_thread_mutex_lock(sub_mutex));
//		check_apr(apr_thread_mutex_unlock(sub_mutex));
		
		char imsg[MAX_MESSAGE_LEN];
		char omsg[MAX_MESSAGE_LEN];
		
		size_t len = 0;
		do {
			memset(imsg, 0, MAX_MESSAGE_LEN);
			len = al_socktube_child_read(sock, imsg, MAX_MESSAGE_LEN);
			while (len) {
				printf("child read %d '%s'\n", len, imsg);
				
				memset(omsg, 0, MAX_MESSAGE_LEN);
				sprintf(omsg, "reply %d \n", childcount++);
				if (al_socktube_child_write(sock, omsg, 1+strlen(omsg))) {
					printf("child write error\n");
				}
				
				memset(imsg, 0, MAX_MESSAGE_LEN);
				len = al_socktube_child_read(sock, imsg, MAX_MESSAGE_LEN);
			}
		} while(len);
		
		apr_thread_yield();
	}
	apr_thread_exit(thread, 0);
	return NULL;
}

void ontick(al_nsec time, void * userdata) {
	
	al_sec t = time * al_time_ns2s;
//	if (!APR_STATUS_IS_EBUSY(apr_thread_mutex_trylock(sub_mutex))) {
//		check_apr(apr_thread_mutex_unlock(sub_mutex));
//	} else {
//		printf("busy\n");
//	}

	char imsg[MAX_MESSAGE_LEN];
	memset(imsg, 0, MAX_MESSAGE_LEN);
	size_t len = al_socktube_parent_read(sock, imsg, MAX_MESSAGE_LEN);
	while (len) {
		printf("parent read %d '%s'\n", len, imsg);
		len = al_socktube_parent_read(sock, imsg, MAX_MESSAGE_LEN);
	}
	
	for (int i=0; i<4; i++) {
		char omsg[MAX_MESSAGE_LEN];
		memset(omsg, 0, MAX_MESSAGE_LEN);
		sprintf(omsg, "sent %d @%f \n", parentcount++, t);
		if (al_socktube_parent_write(sock, omsg, strlen(omsg))) {
			printf("write error\n");
		}
	}
		
	if (t > 9.0) {
		active = 0;
		al_main_exit();
	}
}

void onquit(void * userdata) {
	apr_status_t err;
	
	// wait for subthread to finish:
	check_apr(apr_thread_join(&err, sub_thread));
	
	// free main memory pool:
	apr_pool_destroy(main_mempool);
	
	printf("terminated\n");
}

int main (int argc, char * argv[]) {
	
	// init APR:
	check_apr(apr_initialize());
	atexit(apr_terminate);	// ensure apr tear-down
	
	// create mempool:
	check_apr(apr_pool_initialize());
	check_apr(apr_pool_create(&main_mempool, NULL));
	apr_allocator_max_free_set(apr_pool_allocator_get(main_mempool), 1024);

	sock = al_socktube_create(); 

	// create subthread:
	check_apr(apr_threadattr_create(&sub_thread_attr, main_mempool));
	check_apr(apr_threadattr_detach_set(sub_thread_attr, 0)); // detached was the default
	check_apr(apr_thread_mutex_create(&sub_mutex, APR_THREAD_MUTEX_DEFAULT, main_mempool));
	check_apr(apr_thread_create(&sub_thread, sub_thread_attr, subthreadfunc, NULL, main_mempool));
	

	// enter main loop
	al_main_enter(0.01, ontick, NULL, onquit);

	al_socktube_free(&sock);
	
	return 0;
}