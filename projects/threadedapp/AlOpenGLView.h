#import <Cocoa/Cocoa.h>

@interface AlOpenGLView : NSView {

    NSOpenGLContext * mOpenGLContext;
    NSOpenGLPixelFormat * mPixelFormat;
	
    BOOL mDoubleBuffered;
}

- (id) initWithFrame: (NSRect) frame;
- (id) initWithFrame: (NSRect) frame 
	pixelFormat: (NSOpenGLPixelFormat *) pixelFormat;

- (NSOpenGLContext *) openGLContext;
- (void) prepareOpenGL: (NSOpenGLContext *) context;

- (void) lockOpenGLLock;
- (void) unlockOpenGLLock;

- (void) update;

@end
