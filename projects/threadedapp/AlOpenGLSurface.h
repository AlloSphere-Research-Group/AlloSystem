#import <Cocoa/Cocoa.h>

#import "AlOpenGLView.h"

@interface AlOpenGLSurface : NSObject {
	
	NSOpenGLContext * mOpenGLContext;
    NSOpenGLPixelFormat * mPixelFormat;
	AlOpenGLView * mView;
	
	
}

@end
