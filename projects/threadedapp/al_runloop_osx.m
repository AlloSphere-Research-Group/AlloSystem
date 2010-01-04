#include "al_runloop.h"

struct al_runloop {
	al_sec interval;
	al_runloop_tick_handler tickhandler;
	void * userdata;
	al_runloop_quit_handler quithandler;
	al_nsec t0, logicaltime;		/* birth time (wall clock), scheduler time (logical) */
	
	void * impl;
};


/* period at which the autorelease pool is refreshed */
#define AL_RUNLOOP_GC_INTERVAL (60.0)

/* this is the runloop impl */
@interface AlRunLoop : NSObject {
	NSThread * thread;
	BOOL shouldTerminate;
	struct al_runloop * rl;
}

-(id)initWithStruct:(struct al_runloop *)rl;
-(void)run;
-(void)tick:(id)sender;
-(void)terminate;
-(NSThread *)getThread;

@end

@implementation AlRunLoop

-(id)initWithStruct:(struct al_runloop *)rl;
{
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	
	if (self = [super init])
	{
		self->rl = rl;
		[NSThread detachNewThreadSelector:@selector(run) toTarget:self withObject:NULL];
	}
	
	[pool release];
	return self;
}

/* the thread main function; uses NSRunLoop to handle events & sleeping */
- (void)run {
	// for gc'd applications this isn't necesarry, but doesn't hurt either:
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

	// create a scheduled timer
	[NSTimer scheduledTimerWithTimeInterval:rl->interval target:self selector:@selector(tick:) userInfo:nil repeats:YES];
  
	thread = [NSThread currentThread];
	shouldTerminate = NO;

	BOOL isRunning;
	do {
		// run the loop!
		NSDate * theNextDate = [NSDate dateWithTimeIntervalSinceNow:AL_RUNLOOP_GC_INTERVAL]; 
		isRunning = [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:theNextDate]; 
		// occasionally re-create the autorelease pool whilst program is running
		[pool release];
		pool = [[NSAutoreleasePool alloc] init];            
	} while(isRunning==YES && shouldTerminate==NO);

	// terminated the runloop; free the pool one last time:
	[pool release];
}

-(void)tick:(id)sender {
	al_nsec t = al_time_nsec() - rl->t0;
	rl->logicaltime = t;
	(rl->tickhandler)(t, rl->userdata);
}

-(void)terminate {
	shouldTerminate = YES;
	CFRunLoopStop([[NSRunLoop currentRunLoop] getCFRunLoop]);
}

-(NSThread *)getThread {
	return thread;
}

@end

#pragma mark C API


runloop al_runloop_create(al_sec interval, al_runloop_tick_handler tickhandler, void * userdata, al_runloop_quit_handler quithandler) {
	runloop rl = (runloop)malloc(sizeof(struct al_runloop));
	rl->interval = interval;
	rl->tickhandler = tickhandler;
	rl->quithandler = quithandler;
	rl->userdata = userdata;
	rl->t0 = al_time_nsec();
	rl->logicaltime = 0;
	
	rl->impl = (void *)[[AlRunLoop alloc] initWithStruct: rl];
	return rl;
}

void al_runloop_terminate(runloop rl) {
	AlRunLoop * impl = (AlRunLoop *)(rl->impl);
	[impl performSelector:@selector(terminate) onThread:[impl getThread] withObject:NULL waitUntilDone:YES];
}

// redundant? wouldn't the above work anyway?
void al_runloop_terminate_self(runloop rl) {
	AlRunLoop * impl = (AlRunLoop *)(rl->impl);
	[impl terminate];
}
