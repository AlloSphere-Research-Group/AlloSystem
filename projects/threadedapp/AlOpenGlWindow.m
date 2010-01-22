#import "AlOpenGlWindow.h"


@implementation GUI

- (void)makeWindow
{
    [NSApp setDelegate:self];
	
	
	NSRect frame = NSMakeRect(0, 0, 200, 200);
	NSUInteger styleMask = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask;
	NSRect rect = [NSWindow contentRectForFrameRect:frame styleMask:styleMask];

	window = [[NSWindow alloc] initWithContentRect:rect 
		styleMask:styleMask 
		backing: NSBackingStoreBuffered    
		defer:NO];
	//[window setBackgroundColor:[NSColor blueColor]];
	[window makeKeyAndOrderFront:NSApp];
	
	view = [[AlOpenGLView alloc] init];
	[view prepareOpenGL: [view openGLContext]];
	
	[window setContentView: view];
	[window makeKeyAndOrderFront:self];
}

- (void)makeCurrent
{
    [[view openGLContext] makeCurrentContext];
}

- (void)dealloc
{
    [view release];
    [window release];
    [super dealloc];
}

- (int)width
{
    return [view bounds].size.width;
}

- (int)height
{
    return [view bounds].size.height;
}

- (void)flip
{
    [[view openGLContext] flushBuffer]; 
}

- (void)lock
{
    CGLLockContext((CGLContextObj)[[view openGLContext] CGLContextObj]);
}

- (void)unlock
{
    CGLUnlockContext((CGLContextObj)[[view openGLContext] CGLContextObj]);
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:
    (NSApplication *)theApplication
{
    return YES;
}

@end