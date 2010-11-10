//
//  AppDelegate_Phone.h
//  allo
//
//  Created by Graham Wakefield on 4/14/10.
//  Copyright UCSB 2010. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "alloGLView.h"

@interface AppDelegate_Phone : NSObject <UIApplicationDelegate> {
    UIWindow *window;
	alloGLView *glView;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet alloGLView *glView;

@end

