#include "system/al_config.h"
#include "apr-1/apr_general.h"
#include <stdio.h>
#include <stdlib.h>

static apr_status_t check_apr(apr_status_t err) {
	char errstr[1024];
	if (err != APR_SUCCESS) {
		apr_strerror(err, errstr, 1024);
		fprintf(stderr, errstr);
		exit(0);
	}
	return err;
}

static void initialize(int on) {
	/* safe way to ensure static initialization */
	static int initialized = 0;

	if (on && initialized == 0) {
		check_apr(apr_initialize());
	} else if (!on && initialized == 1) {
		apr_terminate();
	}
	
	initialized = on;
}

int al_initialize() {
	initialize(1);
	return 1;
}

int al_terminate() {
	initialize(0);
	return 0;
}
