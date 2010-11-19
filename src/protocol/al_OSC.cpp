#include <ctype.h> // isgraph
#include <stdio.h> // printf
#include "allocore/protocol/al_OSC.hpp"

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


//void OSCRecv::ProcessMessage(const ReceivedMessage& m, const NetAddr& remote){
//	if(callback) callback(RecvPacket(m, remote, mTime), user);
//}


//struct ReceivedMessage::Impl
//:	public ::osc::ReceivedMessageArgumentStream
//,	public ::osc::ReceivedMessage
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
	return NULL;
}

Recv::Recv(unsigned int port, const char * address, al_sec timeout)
:	SocketRecv(port, address, timeout), mHandler(0), mBuffer(1024), mBackground(false)
{}

int Recv::recv(){
	int r = SocketRecv::recv(&mBuffer[0], mBuffer.size());;
	if(r && mHandler){
		mHandler->parse(&mBuffer[0], r);
	}
	return r;
}

bool Recv::start(){
	mBackground = true;
	return mThread.start(recvThreadFunc, this);
}

/// Stop the background polling
void Recv::stop(){
	//if(mBackground){
		mBackground = false;
	//	mThread.wait();
	//}
}


} // osc::
} // al::

//namespace al{
namespace osc{

void RecvPacket::print(FILE * f) const {
	fprintf(f,"[%15s %d] %s %s\n", remoteIP().c_str(), remotePort(), addr().c_str(), tags().c_str());
}

void RecvPacket::printVerbose(FILE * f) const {
	fprintf(f,"sender:    %s %d\n", remoteIP().c_str(), remotePort());
	fprintf(f,"time:      %lld\n", time);
	fprintf(f,"arguments: %ld\n", argc());
	fprintf(f,"message:   %s %s\n", addr().c_str(), tags().c_str());
}



OSCRecv::OSCRecv(networkRecvCB cb, void * userData)
:	callback(cb), user(userData), mPort(0), mTime(1), mStarted(false)
{}

OSCRecv::OSCRecv(unsigned int port, networkRecvCB cb, void * userData)
:	callback(cb), user(userData), mPort(port), mTime(1), mStarted(false)
{}

OSCRecv::~OSCRecv(){ stop(); }

OSCRecv& OSCRecv::port(unsigned int p){
	mPort = p;
	if(started()){ stop(); start(); }
	return *this;
}

void OSCRecv::ProcessMessage(const OSCMsg& m, const NetAddr& remote){
	if(callback) callback(RecvPacket(m, remote, mTime), user);
}

void OSCRecv::ProcessBundle(const OSCBundle& b, const NetAddr& remote){
	mTime = b.TimeTag();
	OscPacketListener::ProcessBundle(b, remote); // this calls ProcessMessage multiple times
}

void OSCRecv::start(){
	if(started()) return;
	mStarted = true;
	mThread.start(threadFunc, this);
}

void OSCRecv::stop(){
	if(started() && mSocket){
		mSocket->AsynchronousBreak();
		mThread.wait();
	}
	mStarted=false;
}

//THREAD_FUNCTION(OSCRecv::threadFunc){
void * OSCRecv::threadFunc(void * user){
	OSCRecv * oscRecv = (OSCRecv *)user;
	UdpListeningReceiveSocket listeningSocket(
		NetAddr(NetAddr::ANY_ADDRESS, oscRecv->port()), oscRecv);
	oscRecv->mSocket = &listeningSocket;
	listeningSocket.Run();
	oscRecv->mSocket = 0;
	return 0;
}




OSCSend::OSCSend(int maxPacketSizeBytes)
:	mBuffer(0), mStream(0)
{
	maxPacketSize(maxPacketSizeBytes);
}

OSCSend::OSCSend(const char * remoteIP, int port, int maxPacketSizeBytes)
:	mBuffer(0), mStream(0)
{
	maxPacketSize(maxPacketSizeBytes);
	add(remoteIP, port);
}

OSCSend::~OSCSend(){ delete[] mBuffer; delete mStream; }

OSCSend& OSCSend::add(const char * remoteIP, int port){
	mEndpoints.push_back(NetAddr(remoteIP, port));
	return *this;
}

OSCSend& OSCSend::remove(const char * remoteIP, int port){

	NetAddr v(remoteIP, port);
	
	std::vector<NetAddr>::iterator it = mEndpoints.begin();
	for(; it<mEndpoints.end(); it++){
		if(v == *it) mEndpoints.erase(it);
	}
	
	return *this;
}

void OSCSend::maxPacketSize(int bytes){
	if(mBuffer) delete[] mBuffer;
	mBuffer = new char[bytes];
	
	if(mStream) delete mStream;
	mStream = new osc::OutboundPacketStream(mBuffer, bytes);

	mStream->Clear();
}

/// Sends the current outbound packet and then clears it.
void OSCSend::send(){
	//for(int i=0; i<mStream->Size(); ++i) printf("%c", mStream->Data()[i]); printf("\n");
	
//	mSocket.Connect(IpEndpointName("127.0.0.1", 12000));
//	mSocket.Send(mStream->Data(), mStream->Size());
	
	for(unsigned i=0; i<mEndpoints.size(); ++i){
		//printf("%d %d\n", mEndpoints[i].address, mEndpoints[i].port);
		mSocket.SendTo(mEndpoints[i], mStream->Data(), mStream->Size());
	}
	mStream->Clear();
}

} // osc::
//} // al::
