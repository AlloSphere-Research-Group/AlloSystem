#ifndef INCLUDE_AL_OSC_HPP
#define INCLUDE_AL_OSC_HPP

/*
 *	OSC (Open Sound Control) send/receive
 *  AlloSphere Research Group / Media Arts & Technology, UCSB, 2009
 */

/*
	Copyright (C) 2006-2008. The Regents of the University of California (REGENTS). 
	All Rights Reserved.

	Permission to use, copy, modify, distribute, and distribute modified versions
	of this software and its documentation without fee and without a signed
	licensing agreement, is hereby granted, provided that the above copyright
	notice, the list of contributors, this paragraph and the following two paragraphs 
	appear in all copies, modifications, and distributions.

	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
	OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
	BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
	HEREUNDER IS PROVIDED "AS IS". REGENTS HAS  NO OBLIGATION TO PROVIDE
	MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/

/*
Example receive code:

	void onRecv(const osc::RecvPacket& p, void * user){
		printf("sender:    %s %d\n", p.remoteIP().c_str(), p.remotePort());
		printf("time:      %d\n", p.time);
		printf("arguments: %d\n", p.argc());
		printf("message:   %s %s\n", p.addr().c_str(), p.tags().c_str());
	}
	
	OSCRecv r(12000, onRecv);
	r.start();

	
Example send code:

	OSCSend s("127.0.0.1", 12000);

    s << osc::BeginBundleImmediate
        << osc::BeginMessage( "/test1" ) 
            << true << 23 << (float)3.1415 << "hello" << osc::EndMessage
        << osc::BeginMessage( "/test2" ) 
            << true << 24 << (float)10.8 << "world" << osc::EndMessage
        << osc::EndBundle;
		
	s.send();

*/

/*
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

OSC-string "#bundle"			8 bytes				How to know that this data is a bundle
OSC-timetag						8 bytes				Time tag that applies to the entire bundle
Size of first bundle element	int32 = 4 bytes		First bundle element
First bundle element's contents	As many bytes as given by "size of first bundle element"
Size of second bundle element	int32 = 4 bytes		Second bundle element
Second bundle element's contents	As many bytes as given by "size of second bundle element"
etc.												Addtional bundle elements

*/


#include <string>
#include <vector>
#include <stdio.h>
#include "allocore/protocol/oscpack/ip/UdpSocket.h"
#include "allocore/protocol/oscpack/osc/OscOutboundPacketStream.h"
#include "allocore/protocol/oscpack/osc/OscPacketListener.h"
#include "allocore/protocol/oscpack/osc/OscReceivedElements.h"
#include "allocore/protocol/oscpack/osc/OscTypes.h"
#include "allocore/io/al_Socket.hpp"
#include "allocore/system/al_Thread.hpp"

namespace al{
namespace osc{


/// User-defined data
struct Blob{
    Blob(){}
    explicit Blob(const void* data_, unsigned long size_)
		: data( data_ ), size( size_ ){}
    const void* data;
    unsigned long size;
};

/// Time tag

/// Time tags are represented by a 64 bit fixed point number. The first 
/// 32 bits specify the number of seconds since midnight on January 1, 1900,
/// and the last 32 bits specify fractional parts of a second to a precision 
/// of about 200 picoseconds. This is the representation used by Internet 
/// NTP timestamps.The time tag value consisting of 63 zero bits followed by
/// a one in the least signifigant bit is a special case meaning "immediately."
typedef unsigned long long TimeTag;


/// Outbound OSC packet
class Packet{
public:

	/// @param[in] size			size, in bytes, of the packet buffer
	Packet(int size=1024);

	/// @param[in] contents		buffer to copy packet data from
	/// @param[in] size			size, in bytes, of the packet buffer	
	Packet(const char * contents, int size);

	~Packet();

	const char * data() const;		///< Get raw packet data
    bool isMessage() const;			///< Whether packet is a message
    bool isBundle() const;			///< Whether packet is a bundle
	
	/// Pretty-print raw packet bytes
	void printRaw() const;
	
	/// Get number of bytes of current packet data
	int size() const;

	/// Begin a new bundle
	Packet& beginBundle(TimeTag timeTag=1);
	
	/// End bundle
	Packet& endBundle();

	/// Start a new message
	Packet& beginMessage(const std::string& addressPattern);
	
	/// End message
	Packet& endMessage();

	/// Add one argument message
	template <class A>
	Packet& addMessage(const std::string& addr, const A& a){
		beginMessage(addr); (*this)<<a; return endMessage();
	}

	/// Add two argument message
	template <class A, class B>
	Packet& addMessage(const std::string& addr, const A& a, const B& b){
		beginMessage(addr); (*this)<<a<<b; return endMessage();
	}

	/// Add three argument message
	template <class A, class B, class C>
	Packet& addMessage(const std::string& addr, const A& a, const B& b, const C& c){
		beginMessage(addr); (*this)<<a<<b<<c; return endMessage();
	}

	/// Add four argument message
	template <class A, class B, class C, class D>
	Packet& addMessage(const std::string& addr, const A& a, const B& b, const C& c, const D& d){
		beginMessage(addr); (*this)<<a<<b<<c<<d; return endMessage();
	}

	/// Add five argument message
	template <class A, class B, class C, class D, class E>
	Packet& addMessage(const std::string& addr, const A& a, const B& b, const C& c, const D& d, const E& e){
		beginMessage(addr); (*this)<<a<<b<<c<<d<<e; return endMessage();
	}

	Packet& operator<< (int v);					///< Add integer to message
	Packet& operator<< (float v);				///< Add float to message
	Packet& operator<< (double v);				///< Add double to message
	Packet& operator<< (char v);				///< Add char to message
	Packet& operator<< (const char* v);			///< Add C-string to message
	Packet& operator<< (const std::string& v);	///< Add string to message
	Packet& operator<< (const Blob& v);			///< Add Blob to message

	/// Clear current packet contents
	Packet& clear();

protected:
	class Impl; Impl * mImpl;
	std::vector<char> mData;
};


/// Inbound OSC message
class Message{
public:

	/// @param[in] message		raw OSC message bytes
	/// @param[in] size			number of bytes in message
	/// @param[in] timeTag		time tag of message (inherited from bundle)
	Message(const char * message, int size, const TimeTag& timeTag=1);

	/// Pretty-print message information
	void print() const;

	/// Get time tag
	
	/// If the message is contained within a bundle, it will inherit the
	/// time tag of the bundle, otherwise the time tag will be 1 (immediate).
	const TimeTag& timeTag() const { return mTimeTag; }

	/// Get address pattern
	const std::string& addressPattern() const { return mAddressPattern; }

	/// Get type tags
	const std::string& typeTags() const { return mTypeTags; }

	/// Reset stream for converting from raw message bytes to types
	Message& resetStream();

	Message& operator>> (int& v);			///< Extract next stream element as integer
	Message& operator>> (float& v);			///< Extract next stream element as float
	Message& operator>> (double& v);		///< Extract next stream element as double
	Message& operator>> (char& v);			///< Extract next stream element as char
	Message& operator>> (const char*& v);	///< Extract next stream element as C-string
	Message& operator>> (std::string& v);	///< Extract next stream element as string
	Message& operator>> (Blob& v);			///< Extract next stream element as Blob

protected:
	class Impl; Impl * mImpl;
	std::string mAddressPattern;
	std::string mTypeTags;
	TimeTag mTimeTag;
};



/// Iterates through all messages contained within an OSC packet
class PacketHandler{
public:

	virtual ~PacketHandler(){}

	/// Called for each message contained in packet
	virtual void onMessage(Message& m) = 0;

	void parse(const char *packet, int size, TimeTag timeTag=1);
};


class Send : public SocketSend, public Packet{
public:

	/// @param[in] port		Port number
	/// @param[in] address	IP address
	/// @param[in] timeout	< 0: block forever; = 0: no blocking; > 0 block with timeout
	Send(unsigned int port, const char * address = "localhost", al_sec timeout=0)
	:	SocketSend(port, address, timeout)
	{}

	/// Send and clear current packet contents
	int send(){
		int r = SocketSend::send(Packet::data(), Packet::size());
		Packet::clear();
		return r;
	}

	/// Send one argument message immediately
	template <class A>
	int send(const std::string& addr, const A& a){
		addMessage(addr, a); return send();
	}
	
	/// Send two argument message immediately
	template <class A, class B>
	int send(const std::string& addr, const A& a, const B& b){
		addMessage(addr, a,b); return send();
	}

	/// Send three argument message immediately
	template <class A, class B, class C>
	int send(const std::string& addr, const A& a, const B& b, const C& c){
		addMessage(addr, a,b,c); return send();
	}

	/// Send four argument message immediately
	template <class A, class B, class C, class D>
	int send(const std::string& addr, const A& a, const B& b, const C& c, const D& d){
		addMessage(addr, a,b,c,d); return send();
	}

	/// Send five argument message immediately
	template <class A, class B, class C, class D, class E>
	int send(const std::string& addr, const A& a, const B& b, const C& c, const D& d, const E& e){
		addMessage(addr, a,b,c,d,e); return send();
	}

};


class Recv : public SocketRecv{
public:

	/// @param[in] port		Port number
	/// @param[in] address	IP address. If 0, will bind all network interfaces to socket.
	/// @param[in] timeout	< 0: block forever; = 0: no blocking; > 0 block with timeout
	Recv(unsigned int port, const char * address = 0, al_sec timeout=0)
	:	SocketRecv(port, address, timeout), mHandler(0), mBuffer(1024), mBackground(false)
	{}

	bool background() const { return mBackground; }

	/// Set size of internal buffer
	void bufferSize(int n){ mBuffer.resize(n); }

	Recv& handler(PacketHandler& v){ mHandler = &v; return *this; }

	/// Check for OSC packet and call handler
	int recv(){
		int r = SocketRecv::recv(&mBuffer[0], mBuffer.size());;
		if(r && mHandler){
			mHandler->parse(&mBuffer[0], r);
		}
		return r;
	}
	
	void start(){
		mBackground = true;
		mThread.start(sThreadFunc, this);
	}
	
	void stop(){
		//if(mBackground){
			mBackground = false;
		//	mThread.wait();
		//}
	}

protected:
	PacketHandler * mHandler;
	std::vector<char> mBuffer;
	al::Thread mThread;
	bool mBackground;

	static void * sThreadFunc(void * user){
		Recv * r = static_cast<Recv *>(user);
		while(r->background()){
			r->recv();
		}
		return NULL;
	}
};


}
}


//namespace al{
namespace osc{

// Simplified names
typedef ReceivedBundle		OSCBundle;
typedef ReceivedMessage		OSCMsg;
typedef IpEndpointName		NetAddr;


/// Received packet (for callbacks)
struct RecvPacket{
	RecvPacket(const osc::OSCMsg& m, const osc::NetAddr& r, osc::uint64 t)
	:	msg(m), remote(r), time(t){}
	
	unsigned long argc() const { return msg.ArgumentCount(); }
	std::string addr() const { return msg.AddressPattern() ? msg.AddressPattern() : ""; }
	std::string tags() const { return msg.TypeTags() ? msg.TypeTags() : ""; }
	osc::ReceivedMessageArgumentStream args() const { return msg.ArgumentStream(); }
	osc::ReceivedMessage::const_iterator argi() const { return msg.ArgumentsBegin(); }

	void print(FILE * f=stdout) const;
	void printVerbose(FILE * f=stdout) const;
	
	std::string remoteIP() const { char b[32]; remote.AddressAsString(b); return b; }
	int remotePort() const { return remote.port; }

	const osc::OSCMsg& msg;
	const osc::NetAddr& remote;
	osc::uint64 time;
};


/// Open Sound Control Receiver

/// Start its own listening thread and calls callback in its own thread.
///
class OSCRecv : public osc::OscPacketListener {
public:
	typedef void (* networkRecvCB)(const RecvPacket& p, void * user);

	OSCRecv(networkRecvCB cb=0, void * userData=0);
	OSCRecv(unsigned int port, networkRecvCB cb, void * userData=0);

	~OSCRecv();

	networkRecvCB callback;	///< Receive callback
	void * user;			///< User data passed into callback
	
	/// Change port number to receive messages on.
	
	/// If the receiver has already been started, it will be stopped and
	/// restarted using the new port number.
	OSCRecv& port(unsigned int p);
	
	/// Start the OSC receive thread
	void start();
	
	/// Stop the OSC receive thread.
	void stop();
	
	/// Get the receiving port number
	unsigned int port() const { return mPort; }
	
	/// Get whether the receiving thread has been started
	bool started() const { return mStarted; }
	
protected:
	al::Thread mThread;
	unsigned int mPort;
	osc::uint64 mTime;
	UdpListeningReceiveSocket * mSocket;
	bool mStarted;
	
	//static THREAD_FUNCTION(threadFunc);
	static void * threadFunc(void * user);
	virtual void ProcessMessage(const OSCMsg& m, const NetAddr& remote);
	virtual void ProcessBundle(const OSCBundle& b, const NetAddr& remote);
};



/// Open Sound Control Sender
class OSCSend{
public:

	OSCSend(int maxPacketSizeBytes=4096);
	OSCSend(const char * remoteIP, int port, int maxPacketSizeBytes=4096);
	~OSCSend();

	/// Add data to current packet
	template <class T>
	OSCSend& operator << (T data){ (*mStream) << data; return *this; }

	/// Add string to current packet
	OSCSend& operator << (const std::string& v){ (*mStream) << v.c_str(); return *this; }

	/// Add unsigned integer to current packet
	OSCSend& operator << (unsigned int v){ (*mStream) << osc::int32(v); return *this; }
	
	/// Adds a remote endpoint to receive messages
	OSCSend& add(const char * remoteIP, int port);
	
	/// Removes all endpoints
	void clearEndpoints() { mEndpoints.clear(); }

	/// Removes an existing endpoint
	OSCSend& remove(const char * remoteIP, int port);
	
	/// Get endpoints
	std::vector<NetAddr>& endpoints(){ return mEndpoints; }
	const std::vector<NetAddr>& endpoints() const { return mEndpoints; }


	/// Set maximum outbound packet size
	void maxPacketSize(int bytes);
	
	/// Sends the current outbound packet and then clears it.
	void send();

	/// Send address pattern along with 1 argument immediately
	template <class T1>
	void send(const std::string& addressPattern, const T1& arg1);

	/// Send address pattern along with 2 arguments immediately
	template <class T1, class T2>
	void send(const std::string& addressPattern, const T1& arg1, const T2& arg2);
	
	/// Send address pattern along with 3 arguments immediately
	template <class T1, class T2, class T3>
	void send(const std::string& addressPattern, const T1& arg1, const T2& arg2, const T3& arg3);

private:	
	char * mBuffer;
	std::vector<NetAddr> mEndpoints;
	OutboundPacketStream * mStream;
	UdpSocket mSocket;
};



template <class T1>
void OSCSend::send(const std::string& p, const T1& a1){
	((*this) << osc::BeginMessage(p.c_str()) << a1 << osc::EndMessage); send();
}

template <class T1, class T2>
void OSCSend::send(const std::string& p, const T1& a1, const T2& a2){
	((*this) << osc::BeginMessage(p.c_str()) << a1<<a2 << osc::EndMessage); send();
}

template <class T1, class T2, class T3>
void OSCSend::send(const std::string& p, const T1& a1, const T2& a2, const T3& a3){
	((*this) << osc::BeginMessage(p.c_str()) << a1<<a2<<a3 << osc::EndMessage); send();
}


} // osc::
//} // al::
	
#endif
	
