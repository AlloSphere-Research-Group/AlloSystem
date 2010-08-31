#ifndef INCLUDE_AL_OSC_APR_HPP
#define INCLUDE_AL_OSC_APR_HPP 1

/*
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

osc::Send send("localhost", 1000);
send.send("/foo", 1, 2.3, "four");


Example usage (receiving):

osc::Recv recv(1000);
void handler(const osc::ReceivedMessage & p, void * userdata) {
	const std::string tags = p.TypeTags();
	const std::string addr = p.AddressPattern();
	osc::ReceivedMessageArgumentStream args = p.ArgumentStream();
	int i;
	double d;
	std::string s;
	if (tags=="ids") {
		args >> i >> d >> s;
	}
}
recv.recv(handler, NULL);

*/

#include <string>

/* allocore */
#include "system/al_Config.h"
#include "system/al_Time.h"

/* oscpack */
#include "protocol/oscpack/osc/OscOutboundPacketStream.h"
#include "protocol/oscpack/osc/OscReceivedElements.h"

#define OSC_DEFAULT_MAX_MESSAGE_LEN (4096)

namespace osc {



/// Open Sound Control Receiver
/// Supports explicit polling or implicit background thread polling
class Recv {
public:
	typedef void (*MessageParser)(const osc::ReceivedMessage & p, void * userdata);

	class Impl {
	public:
		virtual ~Impl() {}
		virtual size_t recv(char * buffer, size_t maxlen) = 0;
		virtual void start(MessageParser handler, void * userdata, size_t maxlen, al_sec period) {};
		virtual void stop() {};
	};


	Recv(unsigned int port = 7007);
	~Recv();

	unsigned int port() const { return mPort; }

	/*
		Non-blocking explicit poll:
			Using oscpack to call handler per osc::ReceivedMessage
			flushes socket
	*/
	void recv(MessageParser handler, void * userdata = 0, size_t maxlen = OSC_DEFAULT_MAX_MESSAGE_LEN);

	/*
		Non-blocking explicit poll:
			buffer should have at least maxlen bytes of space
			returns the number of bytes received.
			call it repeatedly until it returns zero.
	*/
	size_t recv(char * buffer, size_t maxlen = OSC_DEFAULT_MAX_MESSAGE_LEN);

	/*
		Alternative: polling from a background thread
			(make sure your handler only makes thread-safe calls!)
	*/
	void start(MessageParser handler, void * userdata = 0, size_t maxlen = OSC_DEFAULT_MAX_MESSAGE_LEN, al_sec period=0.001);
	void stop();

	bool background() const { return mBackground; }

protected:
	Impl * mImpl;
	unsigned int mPort;
	bool mBackground;
};


/// Open Sound Control Sender
class Send {
public:
	class Impl {
	public:
		virtual ~Impl() {}
		virtual size_t send(const char * buffer, size_t len) = 0;
	};


	Send(const char * address = "localhost", unsigned int port = 7007, int maxPacketSizeBytes=OSC_DEFAULT_MAX_MESSAGE_LEN);
	~Send();

	/*
		Send a buffer of data
			Returns bytes sent
	*/
	size_t send(const char * buffer, size_t len);

	/*
		Use oscpack's OutboundPacketStream interface to prepare messages and send
			Returns bytes sent
	*/
	size_t send(osc::OutboundPacketStream & packet);

	/// Send address pattern along with 1 argument immediately
	template <class T1>
	size_t send(const std::string& addressPattern, const T1& arg1);

	/// Send address pattern along with 2 arguments immediately
	template <class T1, class T2>
	size_t send(const std::string& addressPattern, const T1& arg1, const T2& arg2);

	/// Send address pattern along with 3 arguments immediately
	template <class T1, class T2, class T3>
	size_t send(const std::string& addressPattern, const T1& arg1, const T2& arg2, const T3& arg3);

	/// Send address pattern along with 4 arguments immediately
	template <class T1, class T2, class T3, class T4>
	size_t send(const std::string& addressPattern, const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4);

	/// Send address pattern along with 5 arguments immediately
	template <class T1, class T2, class T3, class T4, class T5>
	size_t send(const std::string& addressPattern, const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4, const T5& arg5);

	/// Set maximum outbound packet size
	void maxPacketSize(int bytes);

	unsigned int port() const { return mPort; }

protected:
	Impl * mImpl;
	unsigned int mPort;
	char * mBuffer;
	OutboundPacketStream * mStream;
};



/// TODO: Mult-cast Open Sound Control Sender


template <class T1>
size_t Send::send(const std::string& p, const T1& a1){
	(*mStream) << osc::BeginMessage(p.c_str()) << a1 << osc::EndMessage;
	return send((*mStream));
}

template <class T1, class T2>
size_t Send::send(const std::string& p, const T1& a1, const T2& a2){
	(*mStream) << osc::BeginMessage(p.c_str()) << a1<<a2 << osc::EndMessage;
	return send((*mStream));
}

template <class T1, class T2, class T3>
size_t Send::send(const std::string& p, const T1& a1, const T2& a2, const T3& a3){
	(*mStream) << osc::BeginMessage(p.c_str()) << a1<<a2<<a3 << osc::EndMessage;
	return send((*mStream));
}

template <class T1, class T2, class T3, class T4>
size_t Send::send(const std::string& p, const T1& a1, const T2& a2, const T3& a3, const T4& a4){
	(*mStream) << osc::BeginMessage(p.c_str()) << a1<<a2<<a3<<a4 << osc::EndMessage;
	return send((*mStream));
}

template <class T1, class T2, class T3, class T4, class T5>
size_t Send::send(const std::string& p, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5){
	(*mStream) << osc::BeginMessage(p.c_str()) << a1<<a2<<a3<<a4<<a5 << osc::EndMessage;
	return send((*mStream));
}

} // osc::

#endif /* include guard */
