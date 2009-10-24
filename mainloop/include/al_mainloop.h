#ifndef INCLUDE_AL_MAINLOOP_H
#define INCLUDE_AL_MAINLOOP_H

/*
 *  A collection of functions and classes related to application mainloops
 *  AlloSphere Research Group / Media Arts & Technology, UCSB, 2009
 */

/*
	Copyright (C) 2006-2008. The Regents of the University of California (REGENTS). 
	All Rights Reserved.

	Permission to use, copy, modify, distribute, and distribute modified versions
	of this software and its documentation without fee and without a signed
	licensing agreement, is hereby granted, provided that the above copyright
	notice, the list of contributors, this paragraph and the following two paragraphs 
	appear in all copies, modifications, and distributions.

	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
	OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
	BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
	HEREUNDER IS PROVIDED "AS IS". REGENTS HAS  NO OBLIGATION TO PROVIDE
	MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/

#include "al_time.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! 
	get current scheduler (logical) time 
		(zero at al_init())
*/
extern al_nsec al_time();
#define al_now() (al_nsec2sec(al_time()))

typedef void (*main_tick_handler)(al_nsec time, void * userdata);

/*
	Global state in the main loop
*/
typedef struct {
	int isRunning;				/* flag true (1) when in the main loop */
	double interval;			/* in seconds */
	al_nsec t0, logicaltime;		/* birth time (wall clock), scheduler time (logical) */
	main_tick_handler handler;	/* user-supplied event handler */
	void * userdata;			/* passed to the handler */
} al_main_t;

/*!
	Main initialization and termination
*/
extern al_main_t * al_main_init();
extern int al_main_quit();

/*!
	al_main can be used within an existing application mainloop, or can create its own
	
	1. Create a mainloop to take over the main thread (e.g. console tools)
		
		a) Enter the mainloop using:
			al_main_enter()
				This will implicitly call al_init() in case it hadn't already been called.
				This function does not return until al_main_exit() is called.
				Until it returns it makes frequent implicit calls to al_main_tick()
		
		b) Manually trigger the mainloop to exit by calling:
			al_main_exit()
				This will implicitly call al_quit() once the active task has completed,
				and al_main_enter() will return.
	
	2. Using an existing application mainloop with built-in timer (e.g. OSX apps)
	
		a) Attach to the current OS runloop using:
			al_main_attach()
				This will implicitly call al_init() in case it hadn't already been called.
				It attaches a timer to the current OS runloop to implicitly call al_main_tick()
				Returns immediately.
		
		b) When the runloop exits, it is your responsibility to call:
			al_main_quit()
	
	3. Using an existing application mainloop with manual timer (e.g. GLUT apps)
		
		a) Manually initialize delta during startup:
			al_main_init() 
				You must call this before calling al_main_tick()
				
		b) Manually trigger delta at frequent intervals:
			al_main_tick()
				Call this function frequently, e.g. in a draw callback 
				
		c) Manually release delta once the application is closing:
			al_main_quit()
				You must not make any other calls into delta after al_quit()
*/
extern int al_main_enter(double interval, main_tick_handler handler, void * userdata);
extern void al_main_attach(double interval, main_tick_handler handler, void * userdata);
extern void al_main_tick();
extern void al_main_exit();


#ifdef __cplusplus
} // extern "C"
#endif

#endif /* INCLUDE_AL_MAINLOOP_H */