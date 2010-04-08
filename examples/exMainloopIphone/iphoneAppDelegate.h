//
//  iphoneAppDelegate.h
//  iphone
//
//  Created by Graham Wakefield on 4/7/10.
//  Copyright UCSB 2010. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface iphoneAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;

@end

