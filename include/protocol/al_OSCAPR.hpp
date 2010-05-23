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

/* allocore */
#include "system/al_Config.h"
#include "system/al_Time.h"

/* Apache Portable Runtime */
#include "apr-1/apr_general.h"
#include "apr-1/apr_errno.h"
#include "apr-1/apr_pools.h"
#include "apr-1/apr_network_io.h"

/* oscpack */
#include "protocol/oscpack/osc/OscOutboundPacketStream.h"
#include "protocol/oscpack/osc/OscReceivedElements.h"

#define OSC_DEFAULT_MAX_MESSAGE_LEN (4096)

namespace osc {

class Recv {
public:
	typedef void (*MessageParser)(const osc::ReceivedMessage & p, void * userdata);
	
	/* 
		Throws exception on error
	*/
	Recv(unsigned int port = 7007);
	~Recv();
	
	/*
		Call this method to retrieve data from the socket.
		buffer should have at least maxlen bytes of space.
		returns the number of bytes received.
		Call it repeatedly until it returns zero.
		Throws exception on error
	*/
	size_t recv(char * buffer, size_t maxlen = OSC_DEFAULT_MAX_MESSAGE_LEN);
	
	/*
		Similar to above, but uses oscpack to call handler 
			for each osc::ReceivedMessage in the socket.
	*/
	size_t recv(MessageParser handler, void * userdata, size_t maxlen = OSC_DEFAULT_MAX_MESSAGE_LEN);
	
	unsigned int port() { return mPort; }
	
protected:
	apr_pool_t * mPool;
	apr_sockaddr_t * mAddress;
	apr_socket_t * mSock;
	unsigned int mPort;
};
	
class Send {
public:
	/* 
		Throws exception on error
	*/
	Send(const char * address = "localhost", unsigned int port = 7007);
	~Send();
	
	/*
		Send a buffer of data
		Returns bytes sent
		Throws exception on error 
	*/
	size_t send(const char * buffer, size_t len);
	
	/*
		Use oscpack's OutboundPacketStream interface to prepare messages and send
		Returns bytes sent
		Throws exception on error 
	*/
	size_t send(const osc::OutboundPacketStream & packet);
	
	unsigned int port() { return mPort; }
	
protected:
	apr_pool_t * mPool;
	apr_sockaddr_t * mAddress;
	apr_socket_t * mSock;
	unsigned int mPort;
};
	
} // namespace

#endif /* include guard */
