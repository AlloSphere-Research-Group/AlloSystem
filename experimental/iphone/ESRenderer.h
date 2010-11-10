//
//  ESRenderer.h
//  iphone
//
//  Created by Graham Wakefield on 4/7/10.
//  Copyright UCSB 2010. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>

#import <OpenGLES/EAGL.h>
#import <OpenGLES/EAGLDrawable.h>

@protocol GLViewDelegate
- (void)drawView; //:(UIView *)theView;
- (void)setupView:(UIView *)theView;
@end

@protocol ESRenderer <NSObject>

- (void) render:(id <GLViewDelegate>)delegate;
- (BOOL) resizeFromLayer:(CAEAGLLayer *)layer;

@end
