#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>

#import "AlOpenGLView.h"

@interface GUI : NSObject
{
    NSWindow * window;
    AlOpenGLView * view;
}

- (void)makeWindow;
- (void)makeCurrent;

- (void)dealloc;

- (int)width;
- (int)height;

- (void)flip;

- (void)lock;
- (void)unlock;

@end