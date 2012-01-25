#include "al_mDNS.cpp"

#import <Cocoa/Cocoa.h>

// takes a std::string and returns an NSString *
#define CPP_STRING_TO_NSSTRING(STR) [NSString stringWithUTF8String:((STR).c_str())]

@interface ClientDelegate : NSObject <NSNetServiceBrowserDelegate> {
	NSNetServiceBrowser *browser;
    NSNetService *connectedService;
    NSMutableArray *services;
	BOOL isConnected;
}

@end

@implementation ClientDelegate

-(id)initWithDomain:(NSString *) domain type:(NSString *)type {
	if ((self = [super init])){
		services = [NSMutableArray new];
		//browser = [[NSNetServiceBrowser new] autorelease];
		browser = [NSNetServiceBrowser new];
		browser.delegate = self;
		isConnected = NO;
		[browser searchForServicesOfType:type inDomain:@""];
	}
	return self;
}

-(void)search {

}

-(void)dealloc {
	//self.connectedService = nil;
    //self.browser = nil;
	[browser stop];
    [browser release];
	[services release];
	[super dealloc];
}

#pragma mark Net Service Browser Delegate Methods
- (void)netServiceBrowserWillSearch:(NSNetServiceBrowser *)aNetServiceBrowser {
	NSLog(@"netServiceBrowserWillSearch: %@", aNetServiceBrowser);
}
- (void)netServiceBrowserDidStopSearch:(NSNetServiceBrowser *)aNetServiceBrowser {
	NSLog(@"netServiceBrowserDidStopSearch: %@", aNetServiceBrowser);
}
- (void)netServiceBrowser:(NSNetServiceBrowser *)aNetServiceBrowser didNotSearch:(NSDictionary *)errorDict {
	NSLog(@"didNotSearch: %@", errorDict);
}
- (void)netServiceBrowser:(NSNetServiceBrowser *)aNetServiceBrowser didFindDomain:(NSString *)domainString moreComing:(BOOL)moreComing {
	NSLog(@"didFindDomain: %@ %@", domainString, moreComing);
}
- (void)netServiceBrowser:(NSNetServiceBrowser *)aNetServiceBrowser didFindService:(NSNetService *)aNetService moreComing:(BOOL)moreComing {
	NSLog(@"didFindService: %@ %@", aNetService, moreComing);
	
	//[services addObject:aService];

	NSNetService *remoteService = [services objectAtIndex:0];
    remoteService.delegate = self;
    [remoteService resolveWithTimeout:0];
}
- (void)netServiceBrowser:(NSNetServiceBrowser *)aNetServiceBrowser didRemoveDomain:(NSString *)domainString moreComing:(BOOL)moreComing {
	NSLog(@"didRemoveDomain: %@ %@", domainString, moreComing);
}
- (void)netServiceBrowser:(NSNetServiceBrowser *)aNetServiceBrowser didRemoveService:(NSNetService *)aNetService moreComing:(BOOL)moreComing {
	NSLog(@"didRemoveService: %@ %@", aNetService, moreComing);
}

@end



@interface ServerDelegate : NSObject <NSNetServiceDelegate> {
    NSNetService * netService;
}
@end

@implementation ServerDelegate

- (id)initWithDomain:(NSString *)domain type:(NSString *)type name:(NSString *)name port:(int)port
{
	if ((self = [super init])){
		netService = [[NSNetService alloc] 
			initWithDomain:(domain) 
			type:(type) 
			name:(name) 
			port:(port)
		];
		netService.delegate = self;
		[netService publish];
	}
	return self;
}

-(void)dealloc {
    [netService stop];
    [netService release]; 
    netService = nil;
    [super dealloc];
}

#pragma mark Net Service Delegate Methods
-(void)netService:(NSNetService *)aNetService didNotPublish:(NSDictionary *)dict {
    NSLog(@"Failed to publish: %@", dict);
}
- (void)netServiceWillPublish:(NSNetService *)sender {
	NSLog(@"Will publish: %@", sender);
}
- (void)netServiceWillResolve:(NSNetService *)sender { 
	NSLog(@"Will resolve: %@", sender);
}
- (void)netServiceDidResolveAddress:(NSNetService *)sender {
	NSLog(@"netServiceDidResolveAddress: %@", sender);
}
- (void)netService:(NSNetService *)sender didNotResolve:(NSDictionary *)errorDict {
	NSLog(@"netServiceDidResolveAddress: %@", errorDict);
}
- (void)netServiceDidStop:(NSNetService *)sender {
	NSLog(@"netServiceDidStop: %@", sender);
}

@end

class ImplBase {
public:

};


namespace al {
namespace mdns {

#pragma mark private implementation

class Client::Impl : public ImplBase {
public:
	
	Impl(Client * master, const std::string& domain, const std::string& type) : ImplBase(), master(master) {
		NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
		delegate = [[ClientDelegate alloc]
			initWithDomain:CPP_STRING_TO_NSSTRING(domain)
			type:CPP_STRING_TO_NSSTRING(type)
		];
		printf("delegate %p\n", delegate);
		[pool release];
	}
	virtual ~Impl() {
		[delegate release]; 
		delegate = nil;
	}

	void poll(al_sec timeout) {}

	Client * master;
	ClientDelegate * delegate;
};

class Service::Impl : public ImplBase {
public:
	Impl(Service * master, const std::string& name, const std::string& host, uint16_t port, const std::string& type, const std::string& domain) 
	:	ImplBase(), name(name), host(host), type(type), domain(domain), port(port), master(master) {
		NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
		delegate = [[ServerDelegate alloc] 
			initWithDomain:CPP_STRING_TO_NSSTRING(domain) 
			type:CPP_STRING_TO_NSSTRING(type) 
			name:CPP_STRING_TO_NSSTRING(name) 
			port:(port)
		];
		//printf("delegate %p\n", delegate);
		[pool release];
	}
	
	virtual ~Impl() {
		[delegate release]; 
		delegate = nil;
	}

	void poll(al_sec timeout) {}
	
	std::string name, host, type, domain;
	uint16_t port;
	Service * master;
	
	ServerDelegate * delegate;
};

#pragma mark public interface

Client::Client(const std::string& type, const std::string& domain) 
:	type(type), domain(domain) {
	mImpl = new Impl(this, domain, type);
}

Client::~Client() {
	delete mImpl;
}

void Client::poll(al_sec timeout) {
	mImpl->poll(timeout);
}

Service::Service(const std::string& name, uint16_t port, const std::string& type, const std::string& domain) {
	mImpl = new Impl(this, name, Socket::hostIP(), port, type, domain);
}

Service::~Service() {
	delete mImpl;
}

void Service::poll(al_sec timeout) {
	mImpl->poll(timeout);
}

} // ::mdns
} // ::al
