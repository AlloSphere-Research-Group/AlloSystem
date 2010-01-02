
#include <stdio.h>
#include <stdlib.h>

#include "main.h"

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
	for (int i=0; i<10000; i++) {
		check_apr(apr_thread_mutex_lock(sub_mutex));
		printf(".");
		check_apr(apr_thread_mutex_unlock(sub_mutex));
		
		apr_thread_yield();
	}
	apr_thread_exit(thread, 0);
	return NULL;
}

void ontick(al_nsec time, void * userdata) {
	
	al_sec t = time * al_time_ns2s;
	if (!APR_STATUS_IS_EBUSY(apr_thread_mutex_trylock(sub_mutex))) {
		printf("%f\n", t);
		check_apr(apr_thread_mutex_unlock(sub_mutex));
	} else {
		printf("busy\n");
	}
		
	if (t > 3.0) {
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

	apr_status_t err;
	// init APR:
	check_apr(apr_initialize());
	atexit(apr_terminate);	// ensure apr tear-down
	
	// create mempool:
	check_apr(apr_pool_initialize());
	check_apr(apr_pool_create(&main_mempool, NULL));
	apr_allocator_max_free_set(apr_pool_allocator_get(main_mempool), 1024);

	// create subthread:
	check_apr(apr_threadattr_create(&sub_thread_attr, main_mempool));
	check_apr(apr_threadattr_detach_set(sub_thread_attr, 0)); // detached was the default
	check_apr(apr_thread_mutex_create(&sub_mutex, APR_THREAD_MUTEX_DEFAULT, main_mempool));
	check_apr(apr_thread_create(&sub_thread, sub_thread_attr, subthreadfunc, NULL, main_mempool));
	
	// enter main loop
	al_main_enter(0.01, ontick, NULL, onquit);
	return 0;
}