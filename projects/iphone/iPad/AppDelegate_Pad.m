//
//  AppDelegate_Pad.m
//  allo
//
//  Created by Graham Wakefield on 4/14/10.
//  Copyright UCSB 2010. All rights reserved.
//

#import "AppDelegate_Pad.h"

@implementation AppDelegate_Pad

@synthesize window;


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {    
	
    // Override point for customization after application launch
	
    [window makeKeyAndVisible];
	
	return YES;
}


- (void)dealloc {
    [window release];
    [super dealloc];
}


@end
