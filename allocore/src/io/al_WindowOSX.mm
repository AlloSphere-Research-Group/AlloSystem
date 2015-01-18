#include "allocore/io/al_Window.hpp"
#include "allocore/system/al_MainLoop.hpp"	// start/stop loop, rendering
//#include "allocore/system/al_Printing.hpp"	// warnings
#include <stdio.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>

#import <Cocoa/Cocoa.h>

@interface CocoaGLWindow : NSOpenGLView <NSWindowDelegate/*, NSApplicationDelegate*/>
{
	@public al::Window * alloWin;
	@public al::WindowImpl * alloImpl;
	NSWindow * win;

	//system timer, needed to synchronize the frame rate
	NSTimer * renderTimer;
	NSTrackingArea * trackingArea;
}
- (void) create:(NSRect)dims title:(NSString *)title pixelFormat:(NSOpenGLPixelFormat *)format;
- (void) startRenderTimer:(float)fps;
- (void) drawRect: (NSRect)bounds;
- (BOOL) acceptsFirstMouse: (NSEvent *)event;
- (void) mouseClickAll: (NSEvent *)event button:(int)button state:(int)state;
- (void) mouseDraggedAll:(NSEvent *)event button:(int)button;
- (unichar) keyFromEvent:(NSEvent *)event;
- (void) fullScreen:(BOOL)whether;
@end


static int mapMouseButton(int osxButton){
	static int map[] = {al::Mouse::LEFT, al::Mouse::MIDDLE, al::Mouse::RIGHT};
	return (osxButton <= 2) ? map[osxButton] : osxButton;
}


namespace al{

class WindowImpl{
public:
	friend class CocoaGLWindow;

	Window * mAlloWin;
	CocoaGLWindow * mCocoaWin;
	bool mCreated;

	WindowImpl(Window * alloWin)
	:	mCreated(false)
	{
		mCocoaWin = [[CocoaGLWindow alloc] init];
		mCocoaWin->alloWin = alloWin;
		mCocoaWin->alloImpl = this;
		mAlloWin = alloWin;
	}

	// friend of a friend functions :)

	void onDestroy(){
		//mAlloWin->destroy();
		/* Equivalent to:
		if(created()){
			callHandlersOnDestroy();
			implDestroy();
		}
		*/

		if(mAlloWin->created()){
			mAlloWin->callHandlersOnDestroy();
		}
	}

	void onFrame(){
		mAlloWin->callHandlersOnFrame();
	}

	void onResize(int w, int h){
		if(mAlloWin->mDim.w != w || mAlloWin->mDim.h != h){
			mAlloWin->mDim.w = w;
			mAlloWin->mDim.h = h;
			mAlloWin->callHandlersOnResize(w,h);
		}
	}

	void setFullScreenMember(bool v){
		mAlloWin->mFullScreen = v;
	}

	void onVisibility(bool v){
		mAlloWin->mVisible = v;
		mAlloWin->callHandlersOnVisibility(v);
	}

	void onMouseDown(int x, int y, int button){
		Mouse& m = mAlloWin->mMouse;
		m.position(x, y);
		m.button(mapMouseButton(button), true);
		mAlloWin->callHandlersOnMouseDown();
	}

	void onMouseUp(int x, int y, int button){
		Mouse& m = mAlloWin->mMouse;
		m.position(x, y);
		m.button(mapMouseButton(button), false);
		mAlloWin->callHandlersOnMouseUp();
	}

	void onMouseDrag(int x, int y){
		mAlloWin->mMouse.position(x, y);
		mAlloWin->callHandlersOnMouseDrag();
	}

	void onMouseMove(int x, int y){
		mAlloWin->mMouse.position(x, y);
		mAlloWin->callHandlersOnMouseMove();
	}

	void onKeyDown(int key){
		mAlloWin->mKeyboard.setKey(key, true);
		mAlloWin->callHandlersOnKeyDown();
	}

	void onKeyUp(int key){
		mAlloWin->mKeyboard.setKey(key, false);
		mAlloWin->callHandlersOnKeyUp();
	}

	void onKeyModifier(bool cap, bool shf, bool ctl, bool alt, bool met){
		Keyboard& k = mAlloWin->mKeyboard;
		k.caps(cap);
		k.shift(shf);
		k.ctrl(ctl);
		k.alt(alt);
		k.meta(met);
	}
};


void Window::destroyAll(){ //printf("Window::destroyAll\n");
	[NSApp makeWindowsPerform:@selector(close) inOrder:NO];
}


void Window::implCtor(){
	mImpl = new WindowImpl(this);
}

void Window::implDtor(){
	delete mImpl;
}

bool Window::implCreate(){

	// Throttle the allocore loop timer interval to save on CPU.
	// Most events will be handled through the NSApp dispatcher.
	Main::get().interval(1./20);

	// Set main loop to native mode
	Main::get().driver(Main::NATIVE);

	// Make new windows take focus when popping up
	[NSApp activateIgnoringOtherApps:YES];

	// https://developer.apple.com/library/mac///documentation/Cocoa/Reference/ApplicationKit/Classes/NSOpenGLPixelFormat_Class/index.html#//apple_ref/c/tdef/NSOpenGLPixelFormatAttribute
	NSOpenGLPixelFormatAttribute attribs[20];

	unsigned i = 0;
	if(mDisplayMode & DOUBLE_BUF ){	attribs[i++] = NSOpenGLPFADoubleBuffer; }
	if(mDisplayMode & STEREO_BUF ){	attribs[i++] = NSOpenGLPFAStereo; }
	//attribs[i++] = NSOpenGLPFAColorSize; attribs[i++] = 24; // Let OS decide
	if(mDisplayMode & ACCUM_BUF  ){	attribs[i++] = NSOpenGLPFAAccumSize;
									attribs[i++] = 32; }
	if(mDisplayMode & ALPHA_BUF  ){	attribs[i++] = NSOpenGLPFAAlphaSize;
									attribs[i++] = 8; }
	if(mDisplayMode & DEPTH_BUF  ){	attribs[i++] = NSOpenGLPFADepthSize;
									attribs[i++] = 24; }
	if(mDisplayMode & STENCIL_BUF){	attribs[i++] = NSOpenGLPFAStencilSize;
									attribs[i++] = 8; }
	if(mDisplayMode & MULTISAMPLE){	attribs[i++] = NSOpenGLPFAMultisample;
									attribs[i++] = NSOpenGLPFASampleBuffers;
									attribs[i++] = 1;
									attribs[i++] = NSOpenGLPFASamples;
									attribs[i++] = 4;
									attribs[i++] = NSOpenGLPFASampleAlpha; }
	attribs[i] = 0;

	id format = [NSOpenGLPixelFormat alloc];
	[format initWithAttributes: attribs];

	// ensures onResize handlers get called
	int w = mDim.w;
	int h = mDim.h;
	mDim.w = 0;
	mDim.h = 0;

	[mImpl->mCocoaWin
		create: NSMakeRect(mDim.l, mDim.t, w, h)
		title: [NSString stringWithUTF8String:mTitle.c_str()]
		pixelFormat: format
	];

	mImpl->mCreated = true;

	callHandlersOnCreate();

	// Move window to front of the screen list and make it show
	[mImpl->mCocoaWin->win makeKeyAndOrderFront:nil];
	[mImpl->mCocoaWin->win makeMainWindow];

	// Start rendering
	[mImpl->mCocoaWin startRenderTimer:mFPS];

	return true;
}

bool Window::created() const {
	//return [mImpl->mCocoaWin->win windowNumber] > 0;
	return mImpl->mCreated;
}

void Window::implDestroy(){
	[mImpl->mCocoaWin->win close];
}

void Window::implSetDimensions(){
	mImpl->mCocoaWin.frame = NSMakeRect(0, 0, mDim.w, mDim.h);
	int y = [mImpl->mCocoaWin->win screen].frame.size.height - mDim.t;
	[mImpl->mCocoaWin->win setFrameTopLeftPoint:NSMakePoint(mDim.l, y)];
}

void Window::implSetCursor(){
}

void Window::implSetCursorHide(){
	//CGDisplayHideCursor(kCGDirectMainDisplay);
}

void Window::implSetFullScreen(){
	[mImpl->mCocoaWin fullScreen:mFullScreen];
}

void Window::implSetFPS(){
	[mImpl->mCocoaWin->renderTimer invalidate];
	[mImpl->mCocoaWin startRenderTimer:fps()];
}

void Window::implSetTitle(){
	[mImpl->mCocoaWin->win setTitle:[NSString stringWithUTF8String:mTitle.c_str()]];
}

void Window::implSetVSync(){
	// Synchronize buffer swaps with vertical refresh rate
	GLint swapInt = GLint(mVSync);
	[[mImpl->mCocoaWin openGLContext]
		setValues:&swapInt
		forParameter:NSOpenGLCPSwapInterval
	];
}

Window& Window::hide(){
	[NSApp hide:nil];
	return *this;
}

Window& Window::iconify(){
	[mImpl->mCocoaWin->win miniaturize:nil];
	return *this;
}

Window& Window::show(){
	[NSApp unhide:nil];
	return *this;
}

} // al::



@implementation CocoaGLWindow

- (id)init
{
	self = [super init];

	if(self){
		// Creates the shared application instance if not created
		[NSApplication sharedApplication];
		[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
		//[NSApp setDelegate:self];

		// whether view wants an OpenGL backing surface with a resolution
		// greater than 1 pixel per point.
		[self wantsBestResolutionOpenGLSurface];

		win = [NSWindow alloc];

		/*[[NSNotificationCenter defaultCenter]
		 addObserver:self
		 selector:@selector(windowWillClose:)
		 name:NSWindowWillCloseNotification
		 object:win];*/
	}

	return self;
}

- (void) create:(NSRect)dims title:(NSString *)title pixelFormat:(NSOpenGLPixelFormat *)format
{
	self.frame = NSMakeRect(0,0, dims.size.width, dims.size.height);

	//* Used for changing cursor
	trackingArea = [NSTrackingArea alloc];
	[trackingArea initWithRect:dims options:(NSTrackingCursorUpdate | NSTrackingActiveInActiveApp) owner:self userInfo:nil];
	[self addTrackingArea: trackingArea];
	//*/

	[self setPixelFormat:format];

	// Create window
	unsigned int winStyle=
		NSTitledWindowMask|
		NSClosableWindowMask|
		NSMiniaturizableWindowMask|
		NSResizableWindowMask;

	// Note: the content rect is the area without the title bar
	[win
		initWithContentRect:dims
		styleMask:winStyle
		backing:NSBackingStoreBuffered
		defer:NO];

	// Note: can flood event dispatcher (https://developer.apple.com/library/mac/documentation/Cocoa/Conceptual/EventOverview/HandlingMouseEvents/HandlingMouseEvents.html)
	[win setAcceptsMouseMovedEvents:YES];

	[win setTitle:title];
	[win setFrameTopLeftPoint:NSMakePoint(dims.origin.x, [win screen].frame.size.height - dims.origin.y)];
	//[win setAlphaValue: 0.9];
	//[win setHasShadow: NO];

	// Attach view to window
	[win setContentView:self];
	[win makeFirstResponder:self];
	//[win setInitialFirstResponder:view];

	// Adds a fullscreen button to the window title bar and allows calls to
	// NSWindow's toggleFullScreen
	[win setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];

	// Make (NSWindowDelegate) respond to close, occlusion messages, etc.
	[win setDelegate:self];
}

// NSApplicationDelegate
/*- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)app
{
	return YES;
}*/

// NSWindowDelegate
- (void) windowWillClose: (NSNotification *)n
{
	alloImpl->onDestroy();
	alloImpl->mCreated = false;

	unsigned numWins = [[NSApp windows] count];

	//printf("windowWillClose: win=%p, numWins=%d\n", win, numWins);
	//printf("\tchildren=%d\n", (int)[[win childWindows] count]);

	if(1 == numWins){
		// Unset delegate to prevent windowWillClose from being called again by
		// NSApp:terminate.
		[win setDelegate:nil];
		//[NSApp terminate:nil];
		al::Main::get().stop();
	}
}

/*- (void)windowDidBecomeKey:(NSNotification *)n
{	printf("windowDidBecomeKey: %p\n", [n object]); }

- (void)windowDidResignKey:(NSNotification *)n
{	printf("windowDidResignKey: %p\n", [n object]); }*/

// NSWindowDelegate
/*- (void) windowDidResize: (NSNotification *)n
{
	//printf("windowDidResize\n");
}*/

// NSWindowDelegate
- (void)windowDidChangeOcclusionState:(NSNotification *)n
{
	// https://developer.apple.com/library/mac/documentation/Cocoa/Reference/ApplicationKit/Classes/NSWindow_Class/#//apple_ref/c/tdef/NSWindowOcclusionState
	bool visible = [win occlusionState] & NSWindowOcclusionStateVisible;
	//printf("visible %d\n", visible);
	alloImpl->onVisibility(visible);
}

// NSWindowDelegate
- (void)windowDidEnterFullScreen:(NSNotification *)n
{
	//printf("windowDidEnterFullScreen\n");
	alloImpl->setFullScreenMember(true);
}

// NSWindowDelegate
- (void)windowDidExitFullScreen:(NSNotification *)n
{
	//printf("windowDidExitFullScreen\n");
	alloImpl->setFullScreenMember(false);
}

- (void) fullScreen:(BOOL)whether
{
	[win toggleFullScreen:nil];

	//printf("fullscreen %d\n", (([win styleMask] & NSFullScreenWindowMask) == NSFullScreenWindowMask));

	// Note: The NSView fullscreen methods seem to be broken in 10.10.
	// When entering fullscreen, we always get the following warning:
	// "NSWindow warning: adding an unknown subview"
	// [self isInFullScreenMode]
	/*NSDictionary * opts = [NSDictionary dictionaryWithObjectsAndKeys:
		@YES, NSFullScreenModeAllScreens, // cover all screens
		//NSApplicationPresentationAutoHideDock, NSFullScreenModeApplicationPresentationOptions, // auto-hide dock/show menu
		nil];

	if(whether){
		//[win setContentView:self];
		//[win makeFirstResponder:self];
		//printf("parent has %d subviews\n", (int)[[[self superview] subviews] count]);
		//if(![self enterFullScreenMode:[[self window] screen] withOptions:nil]){
		if(![self enterFullScreenMode:[NSScreen mainScreen] withOptions:opts]){
		//if(![self enterFullScreenMode:[[self window] screen] withOptions:opts]){
		}
		//printf("parent has %d subviews\n", (int)[[[self superview] subviews] count]);
		//printf("%d windows after fullscreen\n", (int)[[NSApp windows] count]);
	}
	else{
		[self exitFullScreenModeWithOptions:nil];
		// Strangely, we must call this for view to respond to events again
		[[self window] makeFirstResponder:self];
		// FIXME: lose app quit when clicking close button
		// CAUSE: each time we enter fullscreen, Cocoa creates a new window,
		// but exiting does not destroy the window!
		//[[self window] setContentView:self];
		//[[self window] makeKeyAndOrderFront:nil];
		//[[self window] makeMainWindow];
	}*/
}

/* This method is called only once after the OpenGL context is made the current context. Subclasses that implement this method can use it to configure the Open GL state in preparation for drawing. */
- (void) prepareOpenGL
{
}

-(void) startRenderTimer:(float)fps
{
	float interval = 1./fps;
	renderTimer = [NSTimer timerWithTimeInterval:interval
										  target:self
										selector:@selector(timerFired:)
										userInfo:nil
										 repeats:YES];
	[[NSRunLoop currentRunLoop] addTimer:renderTimer forMode:NSDefaultRunLoopMode];
	// ensure timer fires during resize
	[[NSRunLoop currentRunLoop] addTimer:renderTimer forMode:NSEventTrackingRunLoopMode];

	//printf("started timer\n");
}

// Render timer callback
- (void) timerFired:(id)sender
{
	// flag that window has to be redrawn
	[self setNeedsDisplay:YES];
}

// Each time window has to be redrawn, this method is called
- (void) drawRect:(NSRect)bounds
{
	alloWin->updateFrameTime();
	alloImpl->onFrame();

	// Copy back buffer to front buffer
	[[self openGLContext] flushBuffer];
}

- (void)reshape
{
	int w = [self frame].size.width;
	int h = [self frame].size.height;
	//printf("window reshape w:%3d h:%3d\n", w,h);
	alloImpl->onResize(w,h);
}

- (void) flagsChanged: (NSEvent *)event
{
	// See https://developer.apple.com/library/mac/documentation/Cocoa/Reference/ApplicationKit/Classes/NSEvent_Class/#//apple_ref/doc/constant_group/Modifier_Flags
	unsigned flags = [event modifierFlags];

	bool cap = bool(flags & NSAlphaShiftKeyMask);
	bool shf = bool(flags & NSShiftKeyMask);
	bool ctl = bool(flags & NSControlKeyMask);
	bool alt = bool(flags & NSAlternateKeyMask);
	bool met = bool(flags & NSCommandKeyMask);
	//bool fnc = bool(flags & NSFunctionKeyMask);
	//printf("key special cap:%d shf:%d ctl:%d alt:%d met:%d fnc:%d\n", cap,shf,ctl,alt,met, fnc);
	alloImpl->onKeyModifier(cap,shf,ctl,alt,met);
}

- (unichar) keyFromEvent:(NSEvent *)event
{
	unsigned flags = [event modifierFlags];
	//NSString * chrs = [event characters];
	NSString * chrsNoMod = [event charactersIgnoringModifiers];

	unichar keyChar = [chrsNoMod characterAtIndex:0];

	if(keyChar == 127){ // the normal delete key is really backspace
		keyChar = al::Keyboard::BACKSPACE;
	}

	// Arrow keys
	else if(flags & NSFunctionKeyMask){
		switch(keyChar){
		case NSLeftArrowFunctionKey:	keyChar=al::Keyboard::LEFT; break;
		case NSRightArrowFunctionKey:	keyChar=al::Keyboard::RIGHT; break;
		case NSUpArrowFunctionKey:		keyChar=al::Keyboard::UP; break;
		case NSDownArrowFunctionKey:	keyChar=al::Keyboard::DOWN; break;
		case NSInsertFunctionKey:		keyChar=al::Keyboard::INSERT; break;
		case NSDeleteFunctionKey:		keyChar=al::Keyboard::DELETE; break;
		case NSHomeFunctionKey:			keyChar=al::Keyboard::HOME; break;
		case NSEndFunctionKey:			keyChar=al::Keyboard::END; break;
		case NSPageUpFunctionKey:		keyChar=al::Keyboard::PAGE_UP; break;
		case NSPageDownFunctionKey:		keyChar=al::Keyboard::PAGE_DOWN; break;
		default:;
		}
	}

	return keyChar;
}

// See https://developer.apple.com/library/mac/documentation/Cocoa/Reference/ApplicationKit/Classes/NSEvent_Class/#//apple_ref/doc/constant_group/Function_Key_Unicodes
- (void) keyDown:(NSEvent *)event
{
	alloImpl->onKeyDown([self keyFromEvent:event]);
	//printf("key down %3d (%s) mod:%d\n", [chrs UTF8String][0], [chrs UTF8String], flags);
}

- (void) keyUp:(NSEvent *)event
{
	alloImpl->onKeyUp([self keyFromEvent:event]);
}

- (BOOL) acceptsFirstMouse:(NSEvent *)event
{
	return NO;
	//return YES; // respond to a focusing mouse click
}

- (void) mouseClickAll:(NSEvent *)event button:(int)button state:(int)state
{
	int x = (int)[event locationInWindow].x;
	int y = [self frame].size.height - 1 - (int)[event locationInWindow].y;
	// Note: only 1 click count for all buttons
	//int clicks = (int)[event clickCount];
	//printf("mouse click x:%d y:%d b:%d,%d c:%d\n", x,y, button, state, clicks);

	if(state){
		alloImpl->onMouseDown(x,y,mapMouseButton(button));
	}
	else{
		alloImpl->onMouseUp(x,y,mapMouseButton(button));
	}
}
- (void)      mouseDown:(NSEvent *)e{ [self mouseClickAll:e button:0 state:1]; }
- (void)        mouseUp:(NSEvent *)e{ [self mouseClickAll:e button:0 state:0]; }
- (void) rightMouseDown:(NSEvent *)e{ [self mouseClickAll:e button:2 state:1]; }
- (void)   rightMouseUp:(NSEvent *)e{ [self mouseClickAll:e button:2 state:0]; }
- (void) otherMouseDown:(NSEvent *)e{ [self mouseClickAll:e button:1 state:1]; }
- (void)   otherMouseUp:(NSEvent *)e{ [self mouseClickAll:e button:1 state:0]; }

// https://developer.apple.com/library/mac/documentation/Cocoa/Reference/ApplicationKit/Classes/NSEvent_Class/index.html#//apple_ref/occ/instm/NSEvent/scrollingDeltaX
- (void) scrollWheel:(NSEvent *)event
{
	/*float dx = (float)[event scrollingDeltaX];
	float dy = (float)[event scrollingDeltaY];
	int mom = (int)[event momentumPhase];*/

	//printf("wheel dx:%f dy:%f mom:%d\n", dx, dy, mom);
}

- (void) mouseDraggedAll:(NSEvent *)event button:(int)button
{
	int x = (int)[event locationInWindow].x;
	int y = [self frame].size.height - 1 - (int)[event locationInWindow].y;
	//printf("mouse drag x:%d y:%d b:%d\n", x,y, button);

	alloImpl->onMouseDrag(x,y);
}
- (void)      mouseDragged:(NSEvent *)e { [self mouseDraggedAll:e button:0]; }
- (void) rightMouseDragged:(NSEvent *)e { [self mouseDraggedAll:e button:2]; }
- (void) otherMouseDragged:(NSEvent *)e { [self mouseDraggedAll:e button:1]; }

- (void) mouseMoved:(NSEvent *)event
{
	int x = (int)[event locationInWindow].x;
	int y = [self frame].size.height - 1 - (int)[event locationInWindow].y;
	//printf("mouse move x:%d y:%d\n", x,y);

	alloImpl->onMouseMove(x,y);
}

- (void) updateTrackingAreas
{
	//[trackingArea rect] = [self frame];
	//printf("%d\n", [self bounds].size.width);
}

-(void) cursorUpdate: (NSEvent *)event
{
	//int w = [event trackingArea].rect.size.width;
	//printf("%d\n", w);
    //[[NSCursor crosshairCursor] set];
}
@end
