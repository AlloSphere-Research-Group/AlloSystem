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


#include <string>
#include <vector>
#include <stdio.h>
#include "protocol/oscpack/ip/UdpSocket.h"
#include "protocol/oscpack/osc/OscOutboundPacketStream.h"
#include "protocol/oscpack/osc/OscPacketListener.h"
#include "protocol/oscpack/osc/OscReceivedElements.h"
#include "protocol/oscpack/osc/OscTypes.h"
#include "system/al_Thread.hpp"

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
	
	static THREAD_FUNCTION(threadFunc);
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
	
