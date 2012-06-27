/*
	A scheduled callback using NSTimer, which runs in the NSApplication main loop
*/

#import <Cocoa/Cocoa.h>

#include "allocore/system/al_mainloop.hpp"


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



extern "C" void al_main_native_attach(al_sec interval) {
	if (!gClock) {
		gClock = [[OSClock alloc] initWithInterval:interval];	
	}
}

extern "C" void al_main_native_enter(al_sec interval) {
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	// create a new runloop if it doesn't exist:
	NSRunLoop * runloop = [NSRunLoop currentRunLoop];
	
	al::Main& main = al::Main::get();
	al_main_native_attach(interval);
	
	// main loop:
    while (main.isRunning() && [runloop runMode: NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:main.interval()]]) { /*printf(".");*/ }
	
	// done
	[pool release];
	[gClock dealloc];
}

extern "C" void al_main_native_init() {
}
