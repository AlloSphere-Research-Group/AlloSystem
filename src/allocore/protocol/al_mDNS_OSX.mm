#include "al_mDNS.cpp"

#import <Cocoa/Cocoa.h>
#import <sys/socket.h>
#import <netinet/in.h>
#import <arpa/inet.h>

// takes a std::string and returns an NSString *
#define CPP_STRING_TO_NSSTRING(STR) [NSString stringWithUTF8String:((STR).c_str())]

@interface ClientDelegate : NSObject <NSNetServiceBrowserDelegate> {
	NSNetServiceBrowser *browser;
    NSNetService *connectedService;
    NSMutableArray *services;
	BOOL isConnected;
	NSRunLoop *loop;
}

-(void)search;

@end

@implementation ClientDelegate

-(id)initWithDomain:(NSString *) domain type:(NSString *)type {
	if ((self = [super init])){
		services = [NSMutableArray new];
		NSLog(@"allocated browsersssszzzzzzzzzzzzz");
		//browser = [[NSNetServiceBrowser new] autorelease];
		browser = [NSNetServiceBrowser new];
		browser.delegate = self;
		//loop = [[NSRunLoop alloc] init];
		
		[browser searchForServicesOfType:@"_osc._udp" inDomain:@""];

		[[NSRunLoop currentRunLoop] run];
		//[browser scheduleInRunLoop:loop forMode:NSDefaultRunLoopMode];
		//[loop run];		
		
		NSLog(@"after starting search!");

		isConnected = NO;
		//[self search];
		//[self performSelector:@selector(search) withObject:nil afterDelay:3];
//		[self performSelector:@selector(search) afterDelay:1];
		//[browser searchForServicesOfType:type inDomain:@""];
	}
	return self;
}

-(void)search {
	NSLog(@"searching");
}

-(void)dealloc {
	NSLog(@"DEALLOCATED");
	[loop release];
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
	
	[services addObject:aNetService];

	//NSNetService *remoteService = [services objectAtIndex:0];
    aNetService.delegate = self;
    [aNetService resolveWithTimeout:0];
}
- (void)netServiceBrowser:(NSNetServiceBrowser *)aNetServiceBrowser didRemoveDomain:(NSString *)domainString moreComing:(BOOL)moreComing {
	NSLog(@"didRemoveDomain: %@ %@", domainString, moreComing);
}
- (void)netServiceBrowser:(NSNetServiceBrowser *)aNetServiceBrowser didRemoveService:(NSNetService *)aNetService moreComing:(BOOL)moreComing {
	NSLog(@"didRemoveService: %@ %@", aNetService, moreComing);
}

-(void)netServiceDidResolveAddress:(NSNetService *)service {
	NSLog(@"CONNECTED");
	NSLog(service.domain);
    //self.isConnected = YES;
    //self.connectedService = service;
	
	NSString *name = nil;
	int port;
	
	for(NSData *d in [service addresses]) {
		struct sockaddr_in *socketAddress  = (struct sockaddr_in *) [d bytes];
		name = [service name];
		socketAddress = (struct sockaddr_in *)[d bytes];
		char * ipaddress = inet_ntoa(socketAddress->sin_addr);
		NSString *ipString = [NSString stringWithFormat: @"%s", ipaddress];	
	
		//ipString = [NSString stringWithFormat: @"%s",inet_ntoa (socketAddress->sin_addr)];
		port = ntohs(socketAddress->sin_port); // ntohs converts from network byte order to host byte order 
		NSLog(@"Server found is %@ %d",ipString,port);
	}
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
		//NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
		delegate = [[ServerDelegate alloc] 
			initWithDomain:CPP_STRING_TO_NSSTRING(domain) 
			type:CPP_STRING_TO_NSSTRING(type) 
			name:CPP_STRING_TO_NSSTRING(name) 
			port:(port)
		];
		//printf("delegate %p\n", delegate);
		//[pool release];
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
	mImpl = new Impl(this, name, Socket::hostName(), port, type, domain);
}

Service::~Service() {
	delete mImpl;
}

void Service::poll(al_sec timeout) {
	mImpl->poll(timeout);
}

} // ::mdns
} // ::al
