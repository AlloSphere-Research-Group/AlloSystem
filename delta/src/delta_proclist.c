#include "delta.h"

#include "stdlib.h"
#include "stdio.h"

proclist delta_proclist_create(int size) {
	proclist x = (proclist)malloc(sizeof(struct delta_proclist));
	if (x) {
		x->first = NULL;
		x->last = NULL;
		x->nextid = 1;
	}
	return x;
}

void delta_proclist_free(proclist * x) {
	/* anything to do? */
}

void delta_proclist_append(proclist x, process p) {
	if (x->last) {
		p->next = NULL;
		x->last->next = p;
		x->last = p;
	} else {
		x->first = x->last = p;
	}
	p->id = x->nextid++;
}

void delta_proclist_prepend(proclist x, process p) {
	if (x->first) {
		p->next = x->first;
		x->first = p;
	} else {
		x->first = x->last = p;
	}
	p->id = x->nextid++;
}

void delta_proclist_remove(proclist x, process p) {
	process n = x->first;
	
	if (x->first == p) {
		x->first = p->next;
		p->next = NULL; /* don't recycle process, since it may persist beyond us */
		return;
	}
	
	while (n) {
		if (n->next == p) {
			n->next = p->next;
			p->next = NULL; /* don't recycle process, since it may persist beyond us */
			return;
		}
		n = n->next;
	}
	p->id = 0;
}

