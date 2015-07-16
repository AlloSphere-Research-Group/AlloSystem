#if defined(__APPLE__) && defined(__MACH__)

#include "allocore/io/al_Bluetooth.hpp"
#include <stdio.h>

//#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
//#import <CoreFoundation/CoreFoundation.h>
//#import <CoreServices/CoreServices.h>
#import <Foundation/NSRunLoop.h>

#import <IOBluetooth/objc/IOBluetoothDevice.h>
#import <IOBluetooth/objc/IOBluetoothDeviceInquiry.h>
#import <IOBluetooth/objc/IOBluetoothHostController.h>
#import <IOBluetooth/objc/IOBluetoothRFCOMMChannel.h>
#import <IOBluetooth/objc/IOBluetoothSDPServiceRecord.h>
#import <IOBluetooth/objc/IOBluetoothSDPUUID.h>
#import <IOBluetooth/IOBluetoothUserLib.h>

@interface InquiryDelegate : NSObject
{
@public BOOL isDiscovering;
}

- (void) deviceInquiryStarted:(IOBluetoothDeviceInquiry*)sender;
- (void) deviceInquiryComplete:(IOBluetoothDeviceInquiry*)sender error:(IOReturn)error aborted:(BOOL)aborted;
- (void) deviceInquiryDeviceFound:(IOBluetoothDeviceInquiry*)sender	device:(IOBluetoothDevice*)device;
@end

@implementation InquiryDelegate
- (void) deviceInquiryDeviceFound:(IOBluetoothDeviceInquiry*)sender	device:(IOBluetoothDevice*)device
{
	//printf("Found ");
	//const BluetoothDeviceAddress* addressPtr = [device address];
	printf("Name:    %s\n",[[device name] UTF8String]);
	printf("Address: %s\n",[[device addressString] UTF8String]);
	printf("Service: %#x\n", [device serviceClassMajor]);
	printf("Device:  %#x, %#x\n", [device deviceClassMajor], [device deviceClassMinor]);
	printf("\n");
}
- (void) deviceInquiryStarted:(IOBluetoothDeviceInquiry*)sender
{
	//printf("Searching for Bluetooth devices...\n");
	isDiscovering = YES;
}
- (void) deviceInquiryComplete:(IOBluetoothDeviceInquiry*)sender error:(IOReturn)error aborted:(BOOL)aborted
{
	isDiscovering = NO;
	CFRunLoopStop(CFRunLoopGetCurrent());
	printf("Search completed: ");
	printf("Found %lu device(s)\n", [[sender foundDevices] count]);
}
@end


@interface RFCOMMDelegate : NSObject<IOBluetoothRFCOMMChannelDelegate>
{
@public al::Bluetooth::Impl * mImpl;
}
// For all methods, see bottom of "IOBluetoothRFCOMMChannel.h"
- (void) rfcommChannelOpenComplete:(IOBluetoothRFCOMMChannel*)rfcommChannel status:(IOReturn)error;

- (void) rfcommChannelClosed:(IOBluetoothRFCOMMChannel*)rfcommChannel;

// Called when data is received on channel
- (void) rfcommChannelData:(IOBluetoothRFCOMMChannel *)rfcommChannel data:(void *)dataPointer length:(size_t)dataLength;

- (void)rfcommChannelWriteComplete:(IOBluetoothRFCOMMChannel*)rfcommChannel refcon:(void*)refcon status:(IOReturn)error;
@end


@interface RFCOMMThread : NSThread
{
@public al::Bluetooth::Impl * mImpl;
@public IOReturn mStatus;
@public int mReqChan;
@public BOOL mHasTriedToOpen;
}
- (id) init;
- (void) main;
- (void) stopRunLoop;
@end



/*
https://developer.apple.com/library/mac/documentation/devicedrivers/conceptual/bluetooth/BT_Intro/BT_Intro.html#//apple_ref/doc/uid/TP30000997-CH213-BAJDAJDJ

https://developer.apple.com/library/mac/documentation/IOBluetooth/Reference/IOBluetoothDevice_reference/translated_content/IOBluetoothDevice.html#//apple_ref/doc/uid/TP40011421-CLSCHIOBluetoothDevice-DontLinkElementID_1
*/
struct al::Bluetooth::Impl{
	
	//static IOBluetoothDeviceInquiry * inq;
	IOBluetoothDevice * mDevice;
	IOBluetoothRFCOMMChannel * mRFCOMMChannel;
	RFCOMMDelegate * mRFCOMMDelegate;
	
	// The thread we open the port on is the same thread that gets recv
	// (rfcommChannelData) callbacks. Also, in order to get port input events,
	// we must call CFRunLoopRun(). Thus, we must do this all on a separate
	// thread so as not to take over the main thread.
	// See http://stackoverflow.com/questions/10923112/iobluetooth-event-delegates-only-execute-in-an-unwanted-nsrunloop
	RFCOMMThread * mThread;

	std::vector<unsigned char> mRecv[2];
	std::vector<unsigned char> * mRecvW;
	std::vector<unsigned char> * mRecvR;

	Impl(){
		mRecvW = &mRecv[0];
		mRecvR = &mRecv[1];
		mDevice = nil;
		mRFCOMMChannel = nil;
		mRFCOMMDelegate = nil;
		
		mThread = [[RFCOMMThread alloc] init];
		mThread->mImpl = this;
	}


	void setDevice(const std::string& addr){
		std::string addrDashed = addr;
		// change bytes 2,5,8,11,14 from ':' to '-'
		for(int i=0; i<5; ++i) addrDashed[3*i+2] = '-';
		mDevice = [IOBluetoothDevice deviceWithAddressString:[NSString stringWithUTF8String:addrDashed.c_str()]];
	}

	bool recv(std::vector<unsigned char>& buf){
		//CFRunLoopRunInMode(kCFRunLoopDefaultMode, 4, false);
		if(mRecvW->size() > 0){
			std::vector<unsigned char> * t = mRecvW;
			mRecvW = mRecvR;
			mRecvR = t;
			buf.assign(mRecvR->begin(), mRecvR->end());
			mRecvR->clear();
			return true;
		}
		return false;
	}
	
	bool send(const unsigned char * buf, unsigned len){
		if ( mRFCOMMChannel != nil )
		{
			unsigned numBytesRemaining = len;
			IOReturn result = kIOReturnSuccess;
			const unsigned char * chunk = buf;
			
			// Get the RFCOMM Channel's MTU.  Each write can only contain up to
			// the MTU size number of bytes.
			BluetoothRFCOMMMTU mtu = [mRFCOMMChannel getMTU];
			
			// Loop through the data until we have no more to send.
			while((result == kIOReturnSuccess) && (numBytesRemaining > 0)){
				// finds how many bytes I can send:
				unsigned numBytesToSend = (numBytesRemaining > mtu) ? mtu : numBytesRemaining;
				
				// This method won't return until the buffer has been passed to the Bluetooth hardware to be sent to the remote device.
				// Alternatively, the asynchronous version of this method could be used which would queue up the buffer and return immediately.
				result = [mRFCOMMChannel writeSync:(void *)chunk length:numBytesToSend];
				
				// Updates the position in the buffer:
				numBytesRemaining -= numBytesToSend;
				chunk += numBytesToSend;
			}
			
			// We are successful only if all the data was sent:
			if((numBytesRemaining == 0) && (result == kIOReturnSuccess)){
				return true;
			}
		}

		return false;
	}
	
	bool opened() const {
		return (mDevice!=nil) && [mDevice isConnected];
	}

	int openRFCOMM(int chan){

		if(!mDevice) return -1;
	
		mRFCOMMDelegate = [[RFCOMMDelegate alloc] init];
		mRFCOMMDelegate->mImpl = this;
		mThread->mReqChan = chan;
		[mThread start];
		while(mThread->mHasTriedToOpen == NO){
			[NSThread sleepForTimeInterval: 0.01f];
		}
		
		IOReturn status = mThread->mStatus;
		
		//IOReturn status = [mDevice openRFCOMMChannelSync:&mRFCOMMChannel withChannelID:rfcommChannelID delegate:mRFCOMMDelegate];
	
		if(kIOReturnSuccess == status)
		{
			// Set serial parameters:
			//[mRFCOMMChannel setSerialParameters:9600 dataBits:8 parity:kBluetoothRFCOMMParityTypeNoParity stopBits:1];
			
			// must explicitly set delegate (bug??)
			if(kIOReturnSuccess != [mRFCOMMChannel setDelegate:mRFCOMMDelegate]){
				//fprintf(stderr,"al::Bluetooth::Impl::openRFCOMMSync - Error registering delegate\n");
			}
			
			[mRFCOMMDelegate retain];
			//[mRFCOMMChannel retain];
			
			// We need to run the main RunLoop to receive data, however,
			// it blocks main. WTF!!!
			
			//mRFCOMMChannel->mDataAvailablePort;
			
			//[[NSRunLoop currentRunLoop] run];
			
			//NSApplicationLoad();    // May be necessary for 10.5 not to crash.
			//[NSApplication sharedApplication];
			//[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow: 0]];
			//CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, false);

			return [mRFCOMMChannel getChannelID];
		}

		return -1;
	}


	bool open(){
		if(!mDevice) return false;

		if(kIOReturnSuccess == [mDevice openConnection]){
			return true;
		}
		return false;
	}
	
	bool close(){
		if(!mDevice) return false;
		
		if(mRFCOMMChannel != nil){ //printf("Closing RFCOMM channel\n");
			[mThread stopRunLoop];

			// This will close the RFCOMM channel and start an inactivity
			// timer to close the baseband connection if no other channels
			// (L2CAP or RFCOMM) are open.
			if(kIOReturnSuccess != [mRFCOMMChannel closeChannel]){
				fprintf(stderr, "al::Bluetooth::Impl::close - Error closing RFCOMMchannel\n");
			}
			
			// Release the channel object since we are done with it and it
			// isn't useful anymore.
			[mRFCOMMChannel release];
			mRFCOMMChannel = nil;
			[mRFCOMMDelegate release];
		}
	
		if(kIOReturnSuccess == [mDevice closeConnection]){
			return true;
		}

		//fprintf(stderr, "al::Bluetooth::Impl::close - Error closing connection\n");
		return false;
	}


	static bool available(){
		/*
		if( IOBluetoothValidateHardware( nil ) != kIOReturnSuccess ){
			printf("Error: No Bluetooth support detected.\n");
			return;
		}//*/
		return [IOBluetoothHostController defaultController];
	}
	
	static void printDevices(){

		NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

		if (!available()) {
			printf("Error: No Bluetooth support detected.\n");
			return;
		}

		InquiryDelegate * inqDel = [[InquiryDelegate alloc] init];
		//[inqDel retain];
		IOBluetoothDeviceInquiry * inq = [IOBluetoothDeviceInquiry inquiryWithDelegate:inqDel];
		//inq = [IOBluetoothDeviceInquiry inquiryWithDelegate:inqDel];
		//[inq setInquiryLength:20];
		//[inq setUpdateNewDeviceNames:NO];

		IOReturn status = [inq start];

		if( status == kIOReturnSuccess )
		{
			//[inq retain];
			//[inqDel retain];
			CFRunLoopRun(); // must be called to process callbacks!
			//CFRunLoopRunInMode(kCFRunLoopDefaultMode, 10, false);
			//CFRunLoopWakeUp(CFRunLoopGetCurrent());
			/*
			//NSApplicationLoad();    // May be necessary for 10.5 not to crash.
			//[NSApplication sharedApplication];
			// This will kick start NSRunLoop, and return immediately without blocking
			[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow: 0]];
			//*/
			
			//while(inqDel->isDiscovering){}
		}
		else
		{
			//[_messageText setObjectValue:@"Idle (Search Failed)."];
			printf("Search failed\n");
		}
		
		

		/*
		//NSArray * devices = [QTCaptureDevice inputDevices];
		NSArray * devices = [QTCaptureDevice inputDevicesWithMediaType:QTMediaTypeVideo];

		unsigned numDevices = [devices count];
		
		for(unsigned i=0; i<numDevices; ++i){
			QTCaptureDevice * dev = [devices objectAtIndex:i];
			
			NSString * name = [dev localizedDisplayName];
			NSString * modelUID = [dev modelUniqueID];
			NSString * UID = [dev uniqueID];
			
			printf("[%2d] %s, %s\n     (UID = %s)\n", i, [name UTF8String], [modelUID UTF8String], [UID UTF8String]);
			
			NSDictionary * attribs = [dev deviceAttributes];
			NSArray * attribKeys = [attribs allKeys];
			for(unsigned k=0; k<[attribKeys count]; ++k){
				NSString * key = [attribKeys objectAtIndex:k];
				bool val = [attribs objectForKey:key];
				printf("  %s = %d\n", [key UTF8String], val);
			}
		}
		*/

		[pool release];
	}
};


@implementation RFCOMMDelegate
- (void) rfcommChannelOpenComplete:(IOBluetoothRFCOMMChannel*)rfcommChannel status:(IOReturn)error
{
	//printf("rfcommChannelOpenComplete\n");
}
- (void) rfcommChannelClosed:(IOBluetoothRFCOMMChannel*)rfcommChannel
{
	//printf("rfcommChannelClosed\n");
}
- (void) rfcommChannelData:(IOBluetoothRFCOMMChannel *)rfcommChannel data:(void *)dataPointer length:(size_t)dataLength
{
	const unsigned char * buf = (const unsigned char *)dataPointer;
	
	//printf("rfcommChannelData:\n");
	/*
	printf("[ ");
	for(unsigned i=0; i<dataLength; ++i){
		printf("%2x ", buf[i]);
	} printf("]\n");
	//*/
	
	for(unsigned i=0; i<dataLength; ++i){
		mImpl->mRecvW->push_back(buf[i]);
	}
}
- (void)rfcommChannelWriteComplete:(IOBluetoothRFCOMMChannel*)rfcommChannel refcon:(void*)refcon status:(IOReturn)error
{
	//printf("rfcommChannelWriteComplete\n");
}
@end


@implementation RFCOMMThread
- (id) init
{
	self = [super init];
	if(self){
		mHasTriedToOpen = NO;
	}
	return self;
}
- (void) main
{
	//printf("started RFCOMM thread\n");
	mHasTriedToOpen = NO;
	mStatus = kIOReturnError;
	
	UInt8 rfcommChannelID;

	// Use SDP (service discovery protocol) to dynamically obtain an RFCOMM channel.
	if(mReqChan <= 0){
	
		//printf("Dynamically obtaining RFCOMM channel...\n");
	
		/*
		UUIDs
		(See https://www.bluetooth.org/en-us/specification/assigned-numbers/service-discovery 
		 and http://people.csail.mit.edu/rudolph/Teaching/Articles/PartOfBTBook.pdf)
	
		To get the full 128-bit UUID from a 16-bit or 32-bit number, take the 
		Bluetooth Base UUID (00000000-0000-1000-8000-00805F9B34FB) and replace 
		the leftmost segment with the 16-bit or 32-bit value.
	
		SDP		0x0001
		RFCOMM	0x0003
		OBEX	0x0008
		*/
		//IOBluetoothSDPUUID * serviceUUID = [IOBluetoothSDPUUID uuid16:kBluetoothSDPUUID16ServiceClassSerialPort];

		// Create an ObjC service UUID object
		unsigned char uuid[16] =
		{ 0x00,0x00,0x00,0x03, 0x00,0x00, 0x10,0x00, 0x80,0x00, 0x00,0x80,0x5F,0x9B,0x34,0xFB };
		IOBluetoothSDPUUID * serviceUUID = [IOBluetoothSDPUUID uuidWithBytes:uuid length:16];
		if(serviceUUID == nil){
			fprintf(stderr,"al::Bluetooth::Impl::openRFCOMMSync - Error creating service UUID\n");
			mHasTriedToOpen = YES;
			return;
		}

		//[mDevice performSDPQuery:nil];

		// Attempt to get a service record
		IOBluetoothSDPServiceRecord * serviceRecord = [mImpl->mDevice getServiceRecordForUUID:serviceUUID];
		if(serviceRecord == nil){
			fprintf(stderr,"al::Bluetooth::Impl::openRFCOMMSync - Error getting service record\n");
			mHasTriedToOpen = YES;
			return;
		}

		// typedef uint16_t	BluetoothSDPServiceAttributeID;
		/*NSArray * attribs = [serviceRecord sortedAttributes];
		printf("Found %d attributes\n", [attribs count]);
		for(int i=0; i<[attribs count]; ++i){
			BluetoothSDPServiceAttributeID aid = [attribs[i] getAttributeID];
			printf("attribute ID %d\n", aid);
		}*/

		// Finally, get an RFCOMM channel
		if(kIOReturnSuccess != [serviceRecord getRFCOMMChannelID:&rfcommChannelID]){
			fprintf(stderr,"al::Bluetooth::Impl::openRFCOMMSync - Error getting RFCOMM channel\n");
			mHasTriedToOpen = YES;
			return;
		}
	}
	else{
		rfcommChannelID = mReqChan;
	}

	//printf("Opening RFCOMM channel: %u\n", rfcommChannelID);
	//rfcommChannelID = 1;

	mStatus = [mImpl->mDevice
		openRFCOMMChannelSync: &mImpl->mRFCOMMChannel
		withChannelID: rfcommChannelID
		delegate: mImpl->mRFCOMMDelegate
	];
	mHasTriedToOpen = YES;
	
	if(kIOReturnSuccess == mStatus){
		[mImpl->mRFCOMMChannel retain];
		CFRunLoopRun();
	}
	else{
		//fprintf(stderr,"al::Bluetooth::Impl::openRFCOMMSync - Error opening RFCOMM channel %d\n", rfcommChannelID);
	}
	/*else{
		NSLog( @"Error: 0x%lx - unable to open RFCOMM channel.\n", mStatus );
	}*/
}
- (void) stopRunLoop
{
	//printf("stopping RFCOMMThread run loop\n");
	CFRunLoopStop(CFRunLoopGetCurrent());
}
@end



namespace al{

Bluetooth::Bluetooth()
:	mImpl(new Impl), mChannel(-1)
{}

Bluetooth::Bluetooth(const Bluetooth& other)
:	mImpl(new Impl),
	mName(other.mName), mAddr(other.mAddr), mClass(other.mClass),
	mChannel(-1)
{
}

Bluetooth::Bluetooth(const std::string& addr)
:	mImpl(new Impl), mAddr(addr)
{
}

Bluetooth::~Bluetooth(){
	close();
	//delete mImpl; // handled by auto_ptr as long as this dtor is defined
}

/*bool Bluetooth::find(Bluetooth& bt, const std::string& address){
	
}*/

bool Bluetooth::opened() const {
	return mImpl->opened();
}

bool Bluetooth::open(const std::string& addr){
	mImpl->setDevice(addr);
	return mImpl->open();
}

bool Bluetooth::openRFCOMM(const std::string& addr, int chan){
	mImpl->setDevice(addr);
	chan = mImpl->openRFCOMM(chan);
	if(chan != -1){
		mAddr = addr;
		mChannel = chan;
		return true;
	}
	else{
		return false;
	}
}

bool Bluetooth::close(){
	return mImpl->close();
}

bool Bluetooth::send(const std::vector<unsigned char>& buf){
	return send(&buf[0], buf.size());
}

bool Bluetooth::send(const unsigned char * buf, unsigned len){
	return mImpl->send(buf, len);
}

bool Bluetooth::recv(std::vector<unsigned char>& buf){
	return mImpl->recv(buf);
}

void Bluetooth::print(FILE * fp){
	fprintf(fp,"Name:    %s\n", mName.c_str());
	fprintf(fp,"Address: %s\n", mAddr.c_str());
	fprintf(fp,"Class:   %#6x\n", mClass);
	//fprintf(fp,"Service: %#x\n", mCoD & SERVICE_MAJOR_MASK);
	//fprintf(fp,"Device:  %#x, %#x\n", mCoD & DEVICE_MAJOR_MASK, mCoD & DEVICE_MINOR_MASK);
	if(mChannel != -1) fprintf(fp,"Channel: %d\n", mChannel);
	fprintf(fp,"\n");
}


bool Bluetooth::available(){
	return Impl::available();
}

void Bluetooth::printDevices(){
	printf("Searching for Bluetooth devices...\n");
	Impl::printDevices();
}

} // al::


#endif

