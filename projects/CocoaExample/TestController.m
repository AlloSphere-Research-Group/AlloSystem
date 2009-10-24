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

void tick(al_nsec t, void * userdata) {
	NSLog(@"time %f\n", al_nsec2sec(t));
}

@implementation TestController
- (id) init
{
  self = [super init];
  if (self != nil) {
	al_main_init();
	al_main_attach(0.01, tick, NULL);
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
