//
//  iphoneAppDelegate.m
//  iphone
//
//  Created by Graham Wakefield on 4/7/10.
//  Copyright UCSB 2010. All rights reserved.
//

#import "iphoneAppDelegate.h"

@implementation iphoneAppDelegate

@synthesize window;


- (void)applicationDidFinishLaunching:(UIApplication *)application {    

    // Override point for customization after application launch
    [window makeKeyAndVisible];
}


- (void)dealloc {
    [window release];
    [super dealloc];
}


@end
