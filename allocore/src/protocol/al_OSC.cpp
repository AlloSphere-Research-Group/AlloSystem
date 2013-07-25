#include <ctype.h> // isgraph
#include <stdio.h> // printf
#include <string.h>
#include "allocore/system/al_Printing.hpp"
#include "allocore/protocol/al_OSC.hpp"

#include "oscpack/osc/OscOutboundPacketStream.h"
#include "oscpack/osc/OscPacketListener.h"
#include "oscpack/osc/OscReceivedElements.h"
#include "oscpack/osc/OscTypes.h"
#include "oscpack/osc/OscException.h"

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

#define OSCTRY(msg, expr) \
	try { \
		expr \
	} catch (::osc::Exception& e) { \
		AL_WARN("OSC error: %s", e.what()); \
	}

namespace al{
namespace osc{

class Packet::Impl : public ::osc::OutboundPacketStream{
public:
	Impl(char * buf, int size)
	:	::osc::OutboundPacketStream(buf, size)
	{}
};


Packet::Packet(int size)
:	mData(size), mImpl(0)
{
	OSCTRY("Packet::Packet", mImpl = new Impl(&mData[0], size);)
}

Packet::Packet(const char * contents, int size)
:	mData(size), mImpl(0)
{
	OSCTRY("Packet::Packet1", 
		memcpy(&mData[0], contents, size);
		mImpl = new Impl(&mData[0], size);
	)
}

Packet::~Packet(){ delete mImpl; }

Packet& Packet::operator<< (int v){ 
	OSCTRY("Packet::<<int",  (*mImpl) << (::osc::int32)v;) 
	return *this; 
}
Packet& Packet::operator<< (unsigned v){ 
	OSCTRY("Packet::<<unsigned",  (*mImpl) << (::osc::int32)v;) 
	return *this; 
}
Packet& Packet::operator<< (float v){ 
	OSCTRY("Packet::<<float",  (*mImpl) << v;) 
	return *this; 
}
Packet& Packet::operator<< (double v){ 
	OSCTRY("Packet::<<double",  (*mImpl) << v;) 
	return *this; 
}
Packet& Packet::operator<< (char v){ 
	OSCTRY("Packet<<char", (*mImpl) << v;) 
	return *this; 
}
Packet& Packet::operator<< (const char * v){ 
	OSCTRY("Packet::<<const char *",  (*mImpl) << v;) 
	return *this; 
}
Packet& Packet::operator<< (const std::string& v){ 
	OSCTRY("Packet::<< string",  (*mImpl) << v.c_str();) 
	return *this; 
}
Packet& Packet::operator<< (const Blob& v){ 
	OSCTRY("Packet::<<Blob",  (*mImpl) << ::osc::Blob(v.data, v.size);) 
	return *this; 
}

Packet& Packet::beginMessage(const std::string& addr){
	OSCTRY("Packet::beginMessage",  (*mImpl) << ::osc::BeginMessage(addr.c_str());)
	return *this;
}

Packet& Packet::endMessage(){
	OSCTRY("Packet::endMessage",   (*mImpl) << ::osc::EndMessage;) return *this;
}

Packet& Packet::beginBundle(TimeTag timeTag){
	OSCTRY("Packet::beginBundle",  (*mImpl) << ::osc::BeginBundle(timeTag);) return *this;
}

Packet& Packet::endBundle(){
	OSCTRY("Packet::endBundle",  (*mImpl) << ::osc::EndBundle;) return *this;
}

Packet& Packet::clear(){ 
	OSCTRY("Packet::clear", mImpl->Clear();) 
	return *this; 
}

const char * Packet::data() const { 
	return mImpl->Data(); 
}

bool Packet::isBundle() const { 
	return ::osc::ReceivedPacket(data(), size()).IsBundle(); 
}
bool Packet::isMessage() const { 
	return ::osc::ReceivedPacket(data(), size()).IsMessage(); 
}

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
	{
          // printf("made an ::osc::ReceivedMessage out of message %p and size %d\n", message, size);
        }
	
	template <class T>
	void operator>> (T& v){ OSCTRY("Message>>", args>>v;) }
	
	::osc::ReceivedMessageArgumentStream args;
};

Message::Message(const char * message, int size, const TimeTag& timeTag)
:	mImpl(new Impl(message, size)), mTimeTag(timeTag)
{
	OSCTRY("Message()", 
		mAddressPattern = mImpl->AddressPattern();
		mTypeTags = mImpl->ArgumentCount() ? mImpl->TypeTags() : "";
		resetStream();
	)
}
	
Message::~Message() {
	OSCTRY("~Message()", delete mImpl;)
}

void Message::print() const {
	OSCTRY("Message::print", 
		printf("%s, %s %" AL_PRINTF_LL "d\n",
			addressPattern().c_str(), typeTags().c_str(), timeTag());
		
		::osc::ReceivedMessageArgumentIterator it = mImpl->ArgumentsBegin();
		
		printf("\targs = (");
		for(unsigned i=0; i<typeTags().size(); ++i){
			char tag = typeTags()[i];
			switch(tag){
				case 'f': {float v = it->AsFloat(); printf("%g", v);} break;
				case 'i': {long v = it->AsInt32(); printf("%ld", v);} break;
				case 'h': {long long v = it->AsInt64(); printf("%" AL_PRINTF_LL "d", v);} break;
				case 'c': {char v = it->AsChar(); printf("'%c' (=%3d)", isprint(v) ? v : ' ', v);} break;
				case 'd': {double v = it->AsDouble(); printf("%g", v);} break;
				case 's': {const char * v = it->AsString(); printf("%s", v);} break;
				case 'b': printf("blob"); break;
				default:  printf("?");
			}
			if(i < (typeTags().size() - 1)) printf(", ");
			++it;
		}
		printf(")\n");
	)
}

Message& Message::resetStream(){ mImpl->args = mImpl->ArgumentStream(); return *this; }
Message& Message::operator>> (int& v){ 
	::osc::int32 r; 
	OSCTRY("Message::resetStream", (*mImpl)>>r;)
	v=r; 
	return *this; 
}
Message& Message::operator>> (float& v){ 
	OSCTRY("Message >> float", (*mImpl)>>v;) 
	return *this; 
}
Message& Message::operator>> (double& v){ 
	OSCTRY("Message >> double", (*mImpl)>>v;) 
	return *this; 
}
Message& Message::operator>> (char& v){ 
	OSCTRY("Message >> char", (*mImpl)>>v;) 
	return *this; 
}
Message& Message::operator>> (const char*& v){ 
	OSCTRY("Message >> const char *", (*mImpl)>>v;)
	return *this; 
}
Message& Message::operator>> (std::string& v){ 
	const char * r; 
	OSCTRY("Message >> string", (*mImpl)>>r;) 
	v=r; 
	return *this; 
}
Message& Message::operator>> (Blob& v){ 
	::osc::Blob b; 
	OSCTRY("Message >> Blob", (*mImpl)>>b;) 
	v.data=b.data; 
	v.size=b.size; 
	return *this; 
}

#ifdef VERBOSE
#include <netinet/in.h>  // for ntohl
#endif

void PacketHandler::parse(const char *packet, int size, TimeTag timeTag){
	OSCTRY("PacketHandler::parse", 
#ifdef VERBOSE	      
	       printf("PacketHandler::parse(size %d, packet %p)\n", size, packet);
	       printf("Data to parse: ");
	       for(int i=0; i<size; ++i) printf("%c", packet[i]); printf("\n");
#endif

		
		// this is the only generic entry point for parsing packets
		::osc::ReceivedPacket p(packet, size);
#ifdef VERBOSE
	       printf("Just made an ::osc::ReceivedPacket that has contents %p and size %d\n",
		      p.Contents(), p.Size());
#endif

		// iterate through all the bundle elements (bundles or messages)
		if(p.IsBundle()){

#ifdef VERBOSE
		  printf("It's a bundle\n");
		  char *afterTimeTag = (char *)packet+16;  // "#bundle\0" plus 8-byte time tag
		  int firstBundleElementSize = ntohl(* ((int *)  afterTimeTag));
		  printf("First bundle element has size %d\n", firstBundleElementSize);
#endif

		  ::osc::ReceivedBundle r(p);
#ifdef VERBOSE
		  printf("Just made an ::osc::ReceivedBundle that has time tag at %p and %d elements\n",
			 r.timeTag_, r.ElementCount() );
#endif

		  
		  ::osc::ReceivedBundleElementIterator it = r.ElementsBegin();
#ifdef VERBOSE
		  printf("Just made an ::osc::ReceivedBundleElementIterator\n");
#endif

			
#ifdef VERBOSE
		  int i = 1;
#endif
		  while(it != r.ElementsEnd()){
		    const ::osc::ReceivedBundleElement& e = *it;
#ifdef VERBOSE
		    printf("Just made an ::osc::ReceivedBundleElement with contents %p and size %d\n",
			   e.Contents(), e.Size());
#endif
		    


#ifdef VERBOSE
		    printf("After it++, the same ::osc::ReceivedBundleElement has contents %p and size %d\n",
                           e.Contents(), e.Size());


		    printf("Parsing bundle element %d\n", i++);
		    printf("Made an ::osc::ReceivedBundleElement out of the iterator.\n");
		    printf("\tcontents: %p\n", e.Contents());
		    printf("\tsize: %d\n",e.Size());
		    printf("\ttimeTag %ld\n", r.TimeTag());
		    printf("\tLet's try to parse it...\n");
#endif
		    parse(e.Contents(), e.Size(), r.TimeTag());

		    it++;
		  }
		}
		else if(p.IsMessage()){
#ifdef VERBOSE
		  printf("Parsing a message\n");
#endif
		  Message m(packet, size, timeTag);
		  onMessage(m);
		}
	       )
}



Send::Send(uint16_t port, const char * address, al_sec timeout)
:	SocketSend(port, address, timeout)
{}

int Send::send(){
	//int r = SocketSend::send(Packet::data(), Packet::size());
	int r = send(*this);
	OSCTRY("Packet::endMessage", Packet::clear();)
	return r;
}

int Send::send(const Packet& p){
	int r = 0;
	OSCTRY("Packet::endMessage", r = SocketSend::send(p.data(), p.size());)
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

Recv::Recv()
:	SocketRecv(), mHandler(0), mBuffer(1024), mBackground(false)
{
  // printf("Entering Recv::Recv()\n");
}


Recv::Recv(uint16_t port, const char * address, al_sec timeout)
:	SocketRecv(port, address, timeout), mHandler(0), mBuffer(1024), mBackground(false)
{
  // printf("Entering Recv::Recv(port=%d, addr=%s)\n", port, address);
}

int Recv::recv(){
	int r;
#ifdef VERBOSE
        printf("Entering Recv::recv() - mBuffer = %p and mBuffer.size() = %d\n", &mBuffer[0], mBuffer.size());
#endif

	
	/*	printf("Here's what's in my buffer before recv...\n");
	for (int i = 0; i < mBuffer.size(); ++i) {
	  if (mBuffer[i] != 0)  printf("Byte %d: %d (%c)\n", i, mBuffer[i], mBuffer[i]);
	}
	*/

	OSCTRY("Packet::endMessage", 
		r = SocketRecv::recv(&mBuffer[0], mBuffer.size());
		if(r && mHandler){
#ifdef VERBOSE                  
		  printf("Recv:recv() Received %d bytes; parsing...\n", r);
#endif
			mHandler->parse(&mBuffer[0], r);
		}
	)

#ifdef VERBOSE
        printf("Exiting Recv::recv() - mBuffer = %p and mBuffer.size() = %d\n", &mBuffer[0], mBuffer.size());
#endif
	return r;
}

bool Recv::start(){
  //  printf("Entering Recv::start()\n");
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
		mThread.join();
	}
}


} // osc::
} // al::
