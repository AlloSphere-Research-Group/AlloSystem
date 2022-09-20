#ifndef INCLUDE_AL_OSC_HPP
#define INCLUDE_AL_OSC_HPP

/*	Allocore --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2012. The Regents of the University of California.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

		Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.

		Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in the
		documentation and/or other materials provided with the distribution.

		Neither the name of the University of California nor the names of its
		contributors may be used to endorse or promote products derived from
		this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.

	File description:
	Objects for reading, writing, sending, and receiving
	OSC (Open Sound Control) packets.

	File author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
	Graham Wakefield, 2010, grrrwaaa@gmail.com

*/

#include <string>
#include <vector>
#include "allocore/io/al_Socket.hpp"
#include "allocore/system/al_Thread.hpp"

namespace al{

/// @addtogroup allocore
/// @{

/// Open Sound Control (OSC) utilities
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

	/// Add zero argument message
	Packet& addMessage(const std::string& addr){
		beginMessage(addr); return endMessage();
	}

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

	/// Add six argument message
	template <class A, class B, class C, class D, class E, class F>
	Packet& addMessage(const std::string& addr, const A& a, const B& b, const C& c, const D& d, const E& e, const F& f){
		beginMessage(addr); (*this)<<a<<b<<c<<d<<e<<f; return endMessage();
	}

	/// Add seven argument message
	template <class A, class B, class C, class D, class E, class F, class G>
	Packet& addMessage(const std::string& addr, const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g){
		beginMessage(addr); (*this)<<a<<b<<c<<d<<e<<f<<g; return endMessage();
	}

	Packet& operator<< (int v);					///< Add integer to message
	Packet& operator<< (unsigned v);			///< Add integer to message
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
	/// @param[in] senderAddr	IP address of sender
	Message(const char * message, int size, const TimeTag& timeTag=1, const char *senderAddr = nullptr);
	~Message();

	/// Pretty-print message information
	void print() const;

	/// Get time tag

	/// If the message is contained within a bundle, it will inherit the
	/// time tag of the bundle, otherwise the time tag will be 1 (immediate).
	const TimeTag& timeTag() const { return mTimeTag; }

	/// Get address pattern
	const std::string& addressPattern() const { return mAddressPattern; }

	const std::string senderAddress() const { return std::string(mSenderAddr); }

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
	char mSenderAddr[32];
};



/// Interface for classes that can be registered as handlers with a osc::Recv server object
class PacketHandler{
public:

	virtual ~PacketHandler(){}

	/// Called for each message contained in packet
	virtual void onMessage(Message& m) = 0;

	/// For backwards compatibility
	void parse(const char *packet, int size, TimeTag timeTag=1) {
		parse(packet, size, timeTag, nullptr);
	}

	void parse(const char *packet, int size, TimeTag timeTag, const char *senderAddr);
};

// FIXME: These two classes should be merged...
/// Interface for classes that can consume OSC messages
/// When you write a class that inherits from PacketHandler, you might want to add a
/// method to register MessageConsumer objects. This allows writing classes that can consume
/// OSC messages with a set OSC prefix and that can notify if they have consumed the message
class MessageConsumer {
public:
	virtual ~MessageConsumer() {}
	/// Returns true if message was consumed by this class
	/// rootOSCPath should be prefixed to the consumer's OSC path
	virtual bool consumeMessage(Message& m, std::string rootOSCPath) = 0;
};


/// Socket for sending OSC packets
class Send : public SocketClient, public Packet{
public:
	Send(){}

	/// @param[in] port		Port number (valid range is 0-65535)
	/// @param[in] address	IP address
	/// @param[in] timeout	< 0: block forever; = 0: no blocking; > 0 block for timeout seconds
	/// @param[in] size 	Packet buffer size
	Send(uint16_t port, const char * address = "localhost", float timeout=0, int size=1024);

	/// Send and clear current packet contents
	int send();

	/// Send a packet
	int send(const Packet& p);

	/// Send zero argument message immediately
	int send(const std::string& addr){
		addMessage(addr); return send();
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

	/// Send six argument message immediately
	template <class A, class B, class C, class D, class E, class F>
	int send(const std::string& addr, const A& a, const B& b, const C& c, const D& d, const E& e, const F& f){
		addMessage(addr, a,b,c,d,e,f); return send();
	}

	/// Send seven argument message immediately
	template <class A, class B, class C, class D, class E, class F, class G>
	int send(const std::string& addr, const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g){
		addMessage(addr, a,b,c,d,e,f,g); return send();
	}
};



/// Socket for receiving OSC packets

/// Supports explicit polling or implicit background thread polling.
///
class Recv : public SocketServer{
public:
	Recv();

	/// @param[in] port		Port number (valid range is 0-65535)
	/// @param[in] address	IP address. If empty, will bind all network interfaces to socket.
	/// @param[in] timeout	< 0: block forever; = 0: no blocking; > 0 block for timeout seconds
	Recv(uint16_t port, const char * address = "", float timeout=0);

	virtual ~Recv() { stop(); }

	/// Whether background polling is activated
	bool background() const { return mBackground; }

	/// Get current received packet data
	const char * data() const { return &mBuffer[0]; }

	/// Set size of internal buffer
	void bufferSize(int n){ mBuffer.resize(n); }

	/// Set packet handling routine
	Recv& handler(PacketHandler& v){ mHandler = &v; return *this; }

	/// Check for an OSC packet and call handler
	/// returns bytes read
	/// note: use while(recv()){} to ensure queue is fully flushed.
	int recv();

	/// Begin a background thread to poll the socket.

	/// The socket timeout controls the polling period.
	/// Returns whether the thread was started successfully.
	bool start();

	/// Stop the background polling
	void stop();

protected:
	PacketHandler * mHandler;
	std::vector<char> mBuffer;
	al::Thread mThread;
	bool mBackground;
};

/// @} // end allocore group

} // osc::
} // al::

#endif
