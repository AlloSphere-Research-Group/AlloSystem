//
//  MainWindow.m
//  CocoaExample
//
//  Created by Graham Wakefield on 10/23/09.
//  Copyright 2009 UCSB. All rights reserved.
//

#import "TestController.h"

#include "stdio.h"
#include "al_mainloop.h"

void ontick(al_nsec time, void * userdata) {
	al_sec t = time * al_time_ns2s;
	NSLog(@"time %f\n", t);
	if (t > 3.0) {
		al_main_exit();
	}
}

void onquit(void * userdata) {
	[[NSApplication sharedApplication] terminate: nil];
}

@implementation TestController
- (id) init
{
  self = [super init];
  if (self != nil) {
	al_main_attach(0.01, ontick, self, onquit);
  }
  return self;
}
 
- (void) awakeFromNib
{
  NSLog(@"awakeFromNib");
}

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
 NSLog(@"quit");
}
@end
