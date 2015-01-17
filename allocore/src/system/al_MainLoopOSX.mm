/*
	A scheduled callback using NSTimer, which runs in the NSApplication main loop
*/

#import <Cocoa/Cocoa.h>

#include "allocore/system/al_MainLoop.hpp"


/*
	A class to receive event callbacks from the OS
*/
@interface OSClock : NSObject {
	NSTimer * timer;
	al::Main * main;
}

- (id)initWithInterval:(al_sec)ti;
- (void)dealloc;
- (void)tick;

@end

@implementation OSClock

-(void)tick
{
	main->tick();
}

- (id)initWithInterval:(al_sec)interval;
{
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

	if ((self = [super init]))
	{
		main = &al::Main::get();
		timer = [NSTimer timerWithTimeInterval:(NSTimeInterval)interval
									target:self
									selector:@selector(tick)
									userInfo:nil
									repeats:YES];

		NSRunLoop * runloop = [NSRunLoop currentRunLoop];

		// attach to the runloop it was created from
		// keep on going even during modal dialogs, window scrolling etc.
		[runloop addTimer:timer forMode:NSEventTrackingRunLoopMode];
		[runloop addTimer:timer forMode:NSModalPanelRunLoopMode];
		[runloop addTimer:timer forMode:NSDefaultRunLoopMode];
	}
	[pool release];
	return self;
}

- (void)dealloc
{
	if(timer) {
		[timer invalidate];
		timer = nil;
	}
    [super dealloc];
}

@end

static OSClock * gClock;


extern "C" void al_main_native_init() {
	// Creates the shared application instance (NSApp) if not created
	[NSApplication sharedApplication];
}

extern "C" void al_main_native_attach(al_sec interval) {
	if (!gClock) {
		gClock = [[OSClock alloc] initWithInterval:interval];
	}
}

extern "C" void al_main_native_enter(al_sec interval) {

	// This will attach a periodic timer to the main loop
	al_main_native_attach(interval);

	// Start main event loop and periodic timers
	[NSApp run];
	//printf("returning from al_main_native_enter\n");
	return;


	// The old way of running the main loop---
	// This required ticking the main loop manually which turns out to be much
	// more costly than using a periodic timer (gClock) and letting [NSApp run]
	// dispatch events.

	/*NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	// create a new runloop if it doesn't exist:
	NSRunLoop * runloop = [NSRunLoop currentRunLoop];

	al::Main& main = al::Main::get();
	al_main_native_attach(interval);

	// main loop:
    while (main.isRunning() && [runloop runMode: NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:main.interval()]])
	{
		//printf(".");
		// Perform non-blocking event poll since we are preempting the normal
		// event loop from running with a call to [NSApp run]. This is necessary
		// for windows to get mouse/keyboard events. Note that this may not be an
		// ideal solution as we will be constantly polling for events.
		// See: http://stackoverflow.com/questions/6732400/cocoa-integrate-nsapplication-into-an-existing-c-mainloop?rq=1
		NSEvent * event;
		while(nil != (event = [NSApp nextEventMatchingMask:NSAnyEventMask untilDate:nil inMode:NSDefaultRunLoopMode dequeue:YES])){
			//printf("MainLoop event: %2d at %.2f\n", (int)[event type], (double)[event timestamp]);
			[NSApp sendEvent: event];
		}
	}

	// done
	[pool release];
	[gClock dealloc];*/
}

extern "C" void al_main_native_stop(){
	[gClock dealloc]; // this will stop the timer
	[NSApp stop:nil]; // exits run loop after last event is processed
}
