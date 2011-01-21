#include <ctype.h> // isgraph
#include <stdio.h> // printf
#include <string.h>
#include "allocore/protocol/al_OSC.hpp"

#include "oscpack/osc/OscOutboundPacketStream.h"
#include "oscpack/osc/OscPacketListener.h"
#include "oscpack/osc/OscReceivedElements.h"
#include "oscpack/osc/OscTypes.h"

/*
Summary of OSC 1.0 spec from http://opensoundcontrol.org

The unit of transmission of OSC is an OSC Packet.

An OSC Packet consists of
	its contents
	a contiguous block of binary data
	its size, the number of 8-bit bytes that comprise the contents

The contents of an OSC packet must be either an OSC Message or an OSC Bundle.
The first byte of the packet's contents unambiguously distinguishes between 
these two alternatives.
The size of an OSC packet is always a multiple of 4.

An OSC message consists of
	1) an OSC Address Pattern
	2) an OSC Type Tag String
	3) zero or more OSC Arguments

An OSC Address Pattern is an OSC-string beginning with the character '/'

An OSC Type Tag String is an OSC-string beginning with the character ',' (comma)
followed by a sequence of characters corresponding exactly to the sequence of 
OSC Arguments in the given message.

i	int32
f	float32
s	OSC-string
b	OSC-blob

Anatomy of a bundle:

OSC-string "#bundle"			8 bytes				How to know that this data is a bundle
OSC-timetag						8 bytes	(int64)		Time tag that applies to the entire bundle
Size of first bundle element	4 bytes (int32)		First bundle element
1st bundle element's contents	size of 1st bundle
Size of second bundle element	4 bytes (int32)		Second bundle element
2nd bundle element's contents	size of 2nd bundle
etc.												Addtional bundle elements

*/

namespace al{
namespace osc{

class Packet::Impl : public ::osc::OutboundPacketStream{
public:
	Impl(char * buf, int size)
	:	::osc::OutboundPacketStream(buf, size)
	{}
};


Packet::Packet(int size)
:	mData(size)
{
	mImpl = new Impl(&mData[0], size);
}

Packet::Packet(const char * contents, int size)
:	mData(size)
{
	memcpy(&mData[0], contents, size);
	mImpl = new Impl(&mData[0], size);
}

Packet::~Packet(){ delete mImpl; }

Packet& Packet::operator<< (int v){ (*mImpl) << (::osc::int32)v; return *this; }
Packet& Packet::operator<< (float v){ (*mImpl) << v; return *this; }
Packet& Packet::operator<< (double v){ (*mImpl) << v; return *this; }
Packet& Packet::operator<< (char v){ (*mImpl) << v; return *this; }
Packet& Packet::operator<< (const char * v){ (*mImpl) << v; return *this; }
Packet& Packet::operator<< (const std::string& v){ (*mImpl) << v.c_str(); return *this; }
Packet& Packet::operator<< (const Blob& v){ (*mImpl) << ::osc::Blob(v.data, v.size); return *this; }

Packet& Packet::beginMessage(const std::string& addressPattern){
	(*mImpl) << ::osc::BeginMessage(addressPattern.c_str()); return *this;
}

Packet& Packet::endMessage(){
	(*mImpl) << ::osc::EndMessage; return *this;
}

Packet& Packet::beginBundle(TimeTag timeTag){
	(*mImpl) << ::osc::BeginBundle(timeTag); return *this;
}

Packet& Packet::endBundle(){
	(*mImpl) << ::osc::EndBundle; return *this;
}

Packet& Packet::clear(){ mImpl->Clear(); return *this; }

const char * Packet::data() const { return mImpl->Data(); }

bool Packet::isBundle() const { return ::osc::ReceivedPacket(data(), size()).IsBundle(); }
bool Packet::isMessage() const { return ::osc::ReceivedPacket(data(), size()).IsMessage(); }

void Packet::printRaw() const {
	for(int i=0; i<size(); ++i){
		unsigned char c = data()[i];
		printf("%2x ", c);
		isgraph(c) ? printf(" %c     ", c) : printf("       ");
		if((i+1)%4 == 0) printf("\n");
	}
}

int Packet::size() const { return mImpl->Size(); }



struct Message::Impl
:	public ::osc::ReceivedMessage
{
	Impl(const char * message, int size)
	:	::osc::ReceivedMessage(::osc::ReceivedPacket(message,size)), args(ArgumentStream())
	{}
	
	template <class T>
	void operator>> (T& v){ args>>v; }
	
	::osc::ReceivedMessageArgumentStream args;
};

Message::Message(const char * message, int size, const TimeTag& timeTag)
:	mImpl(new Impl(message, size)), mTimeTag(timeTag)
{
	mAddressPattern = mImpl->AddressPattern();
	mTypeTags = mImpl->TypeTags();
	resetStream();
}

void Message::print() const {
	printf("%s, %s %lld\n", addressPattern().c_str(), typeTags().c_str(), timeTag());
//	for(int i=0; typeTags().size(); ++i){
//		char c = 
//		switch(){
//		case 'f': printf("");
//		}
//	}
}

Message& Message::resetStream(){ mImpl->args = mImpl->ArgumentStream(); return *this; }
Message& Message::operator>> (int& v){ ::osc::int32 r; (*mImpl)>>r; v=r; return *this; }
Message& Message::operator>> (float& v){ (*mImpl)>>v; return *this; }
Message& Message::operator>> (double& v){ (*mImpl)>>v; return *this; }
Message& Message::operator>> (char& v){ (*mImpl)>>v; return *this; }
Message& Message::operator>> (const char*& v){ (*mImpl)>>v; return *this; }
Message& Message::operator>> (std::string& v){ const char * r; (*mImpl)>>r; v=r; return *this; }
Message& Message::operator>> (Blob& v){ ::osc::Blob b; (*mImpl)>>b; v.data=b.data; v.size=b.size; return *this; }


void PacketHandler::parse(const char *packet, int size, TimeTag timeTag){

	// this is the only generic entry point for parsing packets
	::osc::ReceivedPacket p(packet, size);

	// iterate through all the bundle elements (bundles or messages)
	if(p.IsBundle()){
		::osc::ReceivedBundle r(p);
		
		::osc::ReceivedBundleElementIterator it = r.ElementsBegin();
		
		while(it != r.ElementsEnd()){
			const ::osc::ReceivedBundleElement& e = *it++;
			parse(e.Contents(), e.Size(), r.TimeTag());
		}
		
	}
	else if(p.IsMessage()){
		Message m(packet, size, timeTag);
		onMessage(m);
	}
}



Send::Send(unsigned int port, const char * address, al_sec timeout)
:	SocketSend(port, address, timeout)
{}

int Send::send(){
	int r = SocketSend::send(Packet::data(), Packet::size());
	Packet::clear();
	return r;
}



static void * recvThreadFunc(void * user){
	Recv * r = static_cast<Recv *>(user);
	while(r->background()){
		r->recv();
	}
//	printf("thread done\n");
	return NULL;
}

Recv::Recv(unsigned int port, const char * address, al_sec timeout)
:	SocketRecv(port, address, timeout), mHandler(0), mBuffer(1024), mBackground(false)
{}

int Recv::recv(){
	int r = SocketRecv::recv(&mBuffer[0], mBuffer.size());
	if(r && mHandler){
		mHandler->parse(&mBuffer[0], r);
	}
	return r;
}

bool Recv::start(){
	mBackground = true;
	if (timeout() <= 0) {
		printf("warning (osc::Recv): timeout <= 0 and background polling may eat up your CPU! Set timeout(seconds) to avoid this.\n");
	}
	return mThread.start(recvThreadFunc, this);
}

/// Stop the background polling
void Recv::stop(){
	if(mBackground){
		mBackground = false;
		mThread.wait();
	}
}


} // osc::
} // al::




// OLD al_OSC.hpp

/*
Example receive code:

	void onRecv(const osc::RecvPacket& p, void * user){
		printf("sender:    %s %d\n", p.remoteIP().c_str(), p.remotePort());
		printf("time:      %d\n", p.time);
		printf("arguments: %d\n", p.argc());
		printf("message:   %s %s\n", p.addr().c_str(), p.tags().c_str());
	}
	
	OSCRecv r(12000, onRecv);
	r.start();	// or, call r.recv() periodically in another loop

	
Example send code:

	OSCSend s("127.0.0.1", 12000);

    s << osc::BeginBundleImmediate
        << osc::BeginMessage( "/test1" ) 
            << true << 23 << (float)3.1415 << "hello" << osc::EndMessage
        << osc::BeginMessage( "/test2" ) 
            << true << 24 << (float)10.8 << "world" << osc::EndMessage
        << osc::EndBundle;
		
	s.send();
	s.send("/test1", true, 23, 3.1415f, "hello");

*/

//
////namespace al{
//namespace osc{
//
//// Simplified names
//typedef ReceivedBundle		OSCBundle;
//typedef ReceivedMessage		OSCMsg;
//typedef IpEndpointName		NetAddr;
//
//
///// Received packet (for callbacks)
//struct RecvPacket{
//	RecvPacket(const osc::OSCMsg& m, const osc::NetAddr& r, osc::uint64 t)
//	:	msg(m), remote(r), time(t){}
//	
//	unsigned long argc() const { return msg.ArgumentCount(); }
//	std::string addr() const { return msg.AddressPattern() ? msg.AddressPattern() : ""; }
//	std::string tags() const { return msg.TypeTags() ? msg.TypeTags() : ""; }
//	osc::ReceivedMessageArgumentStream args() const { return msg.ArgumentStream(); }
//	osc::ReceivedMessage::const_iterator argi() const { return msg.ArgumentsBegin(); }
//
//	void print(FILE * f=stdout) const;
//	void printVerbose(FILE * f=stdout) const;
//	
//	std::string remoteIP() const { char b[32]; remote.AddressAsString(b); return b; }
//	int remotePort() const { return remote.port; }
//
//	const osc::OSCMsg& msg;
//	const osc::NetAddr& remote;
//	osc::uint64 time;
//};
//
//
///// Open Sound Control Receiver
//
///// Start its own listening thread and calls callback in its own thread.
/////
//class OSCRecv : public osc::OscPacketListener {
//public:
//	typedef void (* networkRecvCB)(const RecvPacket& p, void * user);
//
//	OSCRecv(networkRecvCB cb=0, void * userData=0);
//	OSCRecv(unsigned int port, networkRecvCB cb, void * userData=0);
//
//	~OSCRecv();
//
//	networkRecvCB callback;	///< Receive callback
//	void * user;			///< User data passed into callback
//	
//	/// Change port number to receive messages on.
//	
//	/// If the receiver has already been started, it will be stopped and
//	/// restarted using the new port number.
//	OSCRecv& port(unsigned int p);
//	
//	/// Start the OSC receive thread
//	void start();
//	
//	/// Stop the OSC receive thread.
//	void stop();
//	
//	/// Get the receiving port number
//	unsigned int port() const { return mPort; }
//	
//	/// Get whether the receiving thread has been started
//	bool started() const { return mStarted; }
//	
//protected:
//	al::Thread mThread;
//	unsigned int mPort;
//	osc::uint64 mTime;
//	UdpListeningReceiveSocket * mSocket;
//	bool mStarted;
//	
//	//static THREAD_FUNCTION(threadFunc);
//	static void * threadFunc(void * user);
//	virtual void ProcessMessage(const OSCMsg& m, const NetAddr& remote);
//	virtual void ProcessBundle(const OSCBundle& b, const NetAddr& remote);
//};
//
//
//
///// Open Sound Control Sender
//class OSCSend{
//public:
//
//	OSCSend(int maxPacketSizeBytes=4096);
//	OSCSend(const char * remoteIP, int port, int maxPacketSizeBytes=4096);
//	~OSCSend();
//
//	/// Add data to current packet
//	template <class T>
//	OSCSend& operator << (T data){ (*mStream) << data; return *this; }
//
//	/// Add string to current packet
//	OSCSend& operator << (const std::string& v){ (*mStream) << v.c_str(); return *this; }
//
//	/// Add unsigned integer to current packet
//	OSCSend& operator << (unsigned int v){ (*mStream) << osc::int32(v); return *this; }
//	
//	/// Adds a remote endpoint to receive messages
//	OSCSend& add(const char * remoteIP, int port);
//	
//	/// Removes all endpoints
//	void clearEndpoints() { mEndpoints.clear(); }
//
//	/// Removes an existing endpoint
//	OSCSend& remove(const char * remoteIP, int port);
//	
//	/// Get endpoints
//	std::vector<NetAddr>& endpoints(){ return mEndpoints; }
//	const std::vector<NetAddr>& endpoints() const { return mEndpoints; }
//
//
//	/// Set maximum outbound packet size
//	void maxPacketSize(int bytes);
//	
//	/// Sends the current outbound packet and then clears it.
//	void send();
//
//	/// Send address pattern along with 1 argument immediately
//	template <class T1>
//	void send(const std::string& addressPattern, const T1& arg1);
//
//	/// Send address pattern along with 2 arguments immediately
//	template <class T1, class T2>
//	void send(const std::string& addressPattern, const T1& arg1, const T2& arg2);
//	
//	/// Send address pattern along with 3 arguments immediately
//	template <class T1, class T2, class T3>
//	void send(const std::string& addressPattern, const T1& arg1, const T2& arg2, const T3& arg3);
//
//private:	
//	char * mBuffer;
//	std::vector<NetAddr> mEndpoints;
//	OutboundPacketStream * mStream;
//	UdpSocket mSocket;
//};
//
//
//
//template <class T1>
//void OSCSend::send(const std::string& p, const T1& a1){
//	((*this) << osc::BeginMessage(p.c_str()) << a1 << osc::EndMessage); send();
//}
//
//template <class T1, class T2>
//void OSCSend::send(const std::string& p, const T1& a1, const T2& a2){
//	((*this) << osc::BeginMessage(p.c_str()) << a1<<a2 << osc::EndMessage); send();
//}
//
//template <class T1, class T2, class T3>
//void OSCSend::send(const std::string& p, const T1& a1, const T2& a2, const T3& a3){
//	((*this) << osc::BeginMessage(p.c_str()) << a1<<a2<<a3 << osc::EndMessage); send();
//}
//
//
//} // osc::
////} // al::


//	OLD al_OSC.cpp
//
////namespace al{
//namespace osc{
//
//void RecvPacket::print(FILE * f) const {
//	fprintf(f,"[%15s %d] %s %s\n", remoteIP().c_str(), remotePort(), addr().c_str(), tags().c_str());
//}
//
//void RecvPacket::printVerbose(FILE * f) const {
//	fprintf(f,"sender:    %s %d\n", remoteIP().c_str(), remotePort());
//	fprintf(f,"time:      %lld\n", time);
//	fprintf(f,"arguments: %ld\n", argc());
//	fprintf(f,"message:   %s %s\n", addr().c_str(), tags().c_str());
//}
//
//
//
//OSCRecv::OSCRecv(networkRecvCB cb, void * userData)
//:	callback(cb), user(userData), mPort(0), mTime(1), mStarted(false)
//{}
//
//OSCRecv::OSCRecv(unsigned int port, networkRecvCB cb, void * userData)
//:	callback(cb), user(userData), mPort(port), mTime(1), mStarted(false)
//{}
//
//OSCRecv::~OSCRecv(){ stop(); }
//
//OSCRecv& OSCRecv::port(unsigned int p){
//	mPort = p;
//	if(started()){ stop(); start(); }
//	return *this;
//}
//
//void OSCRecv::ProcessMessage(const OSCMsg& m, const NetAddr& remote){
//	if(callback) callback(RecvPacket(m, remote, mTime), user);
//}
//
//void OSCRecv::ProcessBundle(const OSCBundle& b, const NetAddr& remote){
//	mTime = b.TimeTag();
//	OscPacketListener::ProcessBundle(b, remote); // this calls ProcessMessage multiple times
//}
//
//void OSCRecv::start(){
//	if(started()) return;
//	mStarted = true;
//	mThread.start(threadFunc, this);
//}
//
//void OSCRecv::stop(){
//	if(started() && mSocket){
//		mSocket->AsynchronousBreak();
//		mThread.wait();
//	}
//	mStarted=false;
//}
//
////THREAD_FUNCTION(OSCRecv::threadFunc){
//void * OSCRecv::threadFunc(void * user){
//	OSCRecv * oscRecv = (OSCRecv *)user;
//	UdpListeningReceiveSocket listeningSocket(
//		NetAddr(NetAddr::ANY_ADDRESS, oscRecv->port()), oscRecv);
//	oscRecv->mSocket = &listeningSocket;
//	listeningSocket.Run();
//	oscRecv->mSocket = 0;
//	return 0;
//}
//
//
//
//
//OSCSend::OSCSend(int maxPacketSizeBytes)
//:	mBuffer(0), mStream(0)
//{
//	maxPacketSize(maxPacketSizeBytes);
//}
//
//OSCSend::OSCSend(const char * remoteIP, int port, int maxPacketSizeBytes)
//:	mBuffer(0), mStream(0)
//{
//	maxPacketSize(maxPacketSizeBytes);
//	add(remoteIP, port);
//}
//
//OSCSend::~OSCSend(){ delete[] mBuffer; delete mStream; }
//
//OSCSend& OSCSend::add(const char * remoteIP, int port){
//	mEndpoints.push_back(NetAddr(remoteIP, port));
//	return *this;
//}
//
//OSCSend& OSCSend::remove(const char * remoteIP, int port){
//
//	NetAddr v(remoteIP, port);
//	
//	std::vector<NetAddr>::iterator it = mEndpoints.begin();
//	for(; it<mEndpoints.end(); it++){
//		if(v == *it) mEndpoints.erase(it);
//	}
//	
//	return *this;
//}
//
//void OSCSend::maxPacketSize(int bytes){
//	if(mBuffer) delete[] mBuffer;
//	mBuffer = new char[bytes];
//	
//	if(mStream) delete mStream;
//	mStream = new osc::OutboundPacketStream(mBuffer, bytes);
//
//	mStream->Clear();
//}
//
///// Sends the current outbound packet and then clears it.
//void OSCSend::send(){
//	//for(int i=0; i<mStream->Size(); ++i) printf("%c", mStream->Data()[i]); printf("\n");
//	
////	mSocket.Connect(IpEndpointName("127.0.0.1", 12000));
////	mSocket.Send(mStream->Data(), mStream->Size());
//	
//	for(unsigned i=0; i<mEndpoints.size(); ++i){
//		//printf("%d %d\n", mEndpoints[i].address, mEndpoints[i].port);
//		mSocket.SendTo(mEndpoints[i], mStream->Data(), mStream->Size());
//	}
//	mStream->Clear();
//}
//
//} // osc::
////} // al::
