
#include "allocore/system/al_Config.h"
#include "allocore/system/al_Info.hpp"

#import <Cocoa/Cocoa.h>
#import <Foundation/NSBundle.h>

// a dummy class to identify the framework
@interface AlloOSInfo : NSObject {
}
@end

@implementation AlloOSInfo
@end

namespace al {

std::string frameworkResourcePath() {
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	
	NSBundle * alloFramework = [NSBundle bundleForClass:[AlloOSInfo class]];
	std::string str = [[alloFramework resourcePath] UTF8String];
	
	[pool release];
	return str;
}

} // al::