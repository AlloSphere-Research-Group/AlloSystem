#include "system/al_config.h"
#include "apr_general.h"
#include <stdio.h>
#include <stdlib.h>

static int initialized = 0;

static apr_status_t check_apr(apr_status_t err) {
	char errstr[1024];
	if (err != APR_SUCCESS) {
		apr_strerror(err, errstr, 1024);
		fprintf(stderr, errstr);
		exit(0);
	}
	return err;
}

int al_initialize() {
	if (initialized) return initialized;
	
	check_apr(apr_initialize());
	initialized = 1;
	
	return initialized;
}

int al_terminate() {
	if (initialized) {
		apr_terminate();
		initialized = 0;
	}
	return initialized;
}
