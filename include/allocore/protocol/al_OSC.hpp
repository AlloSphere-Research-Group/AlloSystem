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

Example usage (sending):

	osc::Send s(1000, "localhost");
	
	// A simple way to send a message
	s.send("/foo", 1, 2.3, "four");
	
	// Or, use a temp object
	osc::Send(1000, "192.168.0.1").send("/foo", 1, 2.3, "four");

	// Sending a fairly complex time-tagged bundle
	osc::TimeTag timeNow = ...;
	osc::TimeTag dt = ...;

	s.beginBundle(timeNow);		
		s.addMessage("/message11", 12345678, 1.f, 1., "hello world!");
		s.addMessage("/message12", 23456789);
		s.beginBundle(timeNow + dt);
			s.addMessage("/message21", 3456789);
			s.beginBundle(timeNow + dt*2);
				s.addMessage("/message31", 456789);
			s.endBundle();
		s.endBundle();
		s.addMessage("/message13", 56789);
	s.endBundle();
	
	s.send();


Example usage (receiving):

	osc::Recv r(1000);

	struct OSCHandler : public osc::PacketHandler{
		void onMessage(osc::Message& m){
			std::string tags = m.typeTags();
			std::string addr = m.addressPattern();
			osc::TimeTag ttag= m.timeTag();

			int i;
			double d;
			std::string s;
			if(addr == "/example" && tags == "ids" ){
				m >> i >> d >> s;
			}
		}
	};
	
	OSCHandler myOSCHandler;
	r.handler(myOSCHandler);

	// Poll socket manually at periodic intervals ...
	r.timeout(0);	// set to be non-blocking
	void myThreadFunc(){
		while (r.recv()) {}	// use while loop to empty queue
	}

	// or, launch an automatic background thread
	r.timeout(1);	// ensure waiting period is greater than 0
	r.start(); 

*/

#include <string>
#include <vector>
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



/// Socket for sending OSC packets
class Send : public SocketSend, public Packet{
public:

	/// @param[in] port		Port number
	/// @param[in] address	IP address
	/// @param[in] timeout	< 0: block forever; = 0: no blocking; > 0 block with timeout
	Send(unsigned int port, const char * address = "localhost", al_sec timeout=0);

	/// Send and clear current packet contents
	int send();
	
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

/// Supports explicit polling or implicit background thread polling
///
class Recv : public SocketRecv{
public:

	/// @param[in] port		Port number
	/// @param[in] address	IP address. If 0, will bind all network interfaces to socket.
	/// @param[in] timeout	< 0: block forever; = 0: no blocking; > 0 block with timeout
	Recv(unsigned int port, const char * address = 0, al_sec timeout=0);
	
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


} // osc::
} // al::
	
#endif
	
