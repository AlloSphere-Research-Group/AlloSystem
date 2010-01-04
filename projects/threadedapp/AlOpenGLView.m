#import "AlOpenGLView.h"

@interface AlOpenGLView (Private)

- (BOOL) isDoubleBuffered: (NSOpenGLPixelFormat *) pixelFormat;

@end

@implementation AlOpenGLView

+ (NSOpenGLPixelFormat *) defaultPixelFormat
{
    NSOpenGLPixelFormatAttribute attribs[] = {0};
    return [[(NSOpenGLPixelFormat *)[NSOpenGLPixelFormat alloc] initWithAttributes:attribs] autorelease];
}

- (id) initWithFrame: (NSRect) frame
{
    return [self initWithFrame: frame
                   pixelFormat: [[self class] defaultPixelFormat]];
}

- (id) initWithFrame: (NSRect) frame
         pixelFormat: (NSOpenGLPixelFormat *) pixelFormat;
{
    self = [super initWithFrame: frame];
    if (self == nil)
        return nil;
		
	printf("AlOpenGLView\n");
        
    mOpenGLContext = nil;
    mPixelFormat = [pixelFormat retain];
    
    mDoubleBuffered = [self isDoubleBuffered: mPixelFormat];

//	mFullScreenOpenGLContext = nil;
//    mFullScreenPixelFormat = nil;
//    mFullScreen = NO;
//    mFullScreenWidth = 800;
//    mFullScreenHeight = 600;
//    mFullScreenRefreshRate = 60;
//    mFadeTime = 0.5f;
    
//    mOpenGLLock = [[NSRecursiveLock alloc] init];
    
    [[NSNotificationCenter defaultCenter]
        addObserver: self
           selector: @selector(surfaceNeedsUpdate:)
               name: NSViewGlobalFrameDidChangeNotification
             object: self];

	mOpenGLContext = [[NSOpenGLContext alloc] initWithFormat:mPixelFormat shareContext:nil];
	

//    mDisplayLink = NULL;
//    mAnimationTimer = nil;
//#if ANIMATE_WITH_DISPLAY_LINK
//    NSLog(@"Animate with display link");
//    [self initDisplayLink];
//#else
//    NSLog(@"Animate with timer");
//#endif
    
    return self;
}

- (BOOL) isDoubleBuffered: (NSOpenGLPixelFormat *) pixelFormat;
{
    GLint value;
    [pixelFormat getValues: &value
              forAttribute: NSOpenGLPFADoubleBuffer
          forVirtualScreen: 0];
    return value == 1? YES : NO;
}

- (void) prepareOpenGL: (NSOpenGLContext *) context;
{
	NSLog(@"prepareOpenGL");
    
//    // set to vbl sync
//	GLint swapInt = 1;
//	[context setValues: &swapInt
//          forParameter: NSOpenGLCPSwapInterval];
		  
		  
    // init GL stuff here
    glEnable(GL_DEPTH_TEST);
    
    glShadeModel(GL_SMOOTH);    
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glPolygonOffset(1.0f, 1.0f);
    
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}

- (void) lockOpenGLLock;
{
#if ANIMATE_WITH_DISPLAY_LINK
    [mOpenGLLock lock];
#endif
	if ([mOpenGLContext view] != self) {
		[mOpenGLContext setView:self];
	}
}

- (void) unlockOpenGLLock;
{
#if ANIMATE_WITH_DISPLAY_LINK
    [mOpenGLLock unlock];
#endif
}


- (NSOpenGLContext *) openGLContext
{
    [self lockOpenGLLock];
    {
        if (mOpenGLContext == nil)
        {
            mOpenGLContext =
                [[NSOpenGLContext alloc] initWithFormat: mPixelFormat
                                           shareContext: nil];
            [mOpenGLContext makeCurrentContext];
            [self prepareOpenGL: mOpenGLContext];
        }
    }
    [self unlockOpenGLLock];
    return mOpenGLContext;
}

// this tells the window manager that nothing behind our view is visible
-(BOOL) isOpaque {
  
  return YES;
}

- (void) surfaceNeedsUpdate: (NSNotification *) notification;
{
    [self update];
}

- (void) update
{
    [self lockOpenGLLock];
    {
        NSOpenGLContext * context = [self openGLContext];
        
        if ([context view] == self)
        {
            [context update];
        }
    }
    [self unlockOpenGLLock];
}

@end
