#ifndef INCLUDE_AL_IO_SOCKET_HPP
#define INCLUDE_AL_IO_SOCKET_HPP 1

/*
 *	Network I/O
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
	What would a basic socket class need?
	
	binding methods for UDP and TCP are very different
	receive-only or send-only or duplex?
*/

#include <string>
#include "system/al_Config.h"

namespace al{


/// Base class for network socket
class Socket{
public:

	/// Get port number
	unsigned int port() const;

	/// Close the socket
	void close();

	/// Set timeout. < 0: block forever; = 0: no blocking; > 0 block with timeout
	void timeout(al_sec v);

	/// Get name of current host
	static std::string hostName();
	
	/// IP address of current host
	static std::string hostIP();

protected:
	
	/// @sender: true if Socket will send
	Socket(unsigned int port, const char * address, al_sec timeout, bool sender);
	virtual ~Socket();

	void open(unsigned int port, const char * address, bool sender);
	size_t recv(char * buffer, size_t maxlen);
	size_t send(const char * buffer, size_t len);

private:
	class Impl; Impl * mImpl;
};



/// Socket for sending data over a network
class SocketSend : public Socket {
public:

	/// @param[in] port		Port number
	/// @param[in] address	IP address
	/// @param[in] timeout	< 0: block forever; = 0: no blocking; > 0 block with timeout
	SocketSend(unsigned int port, const char * address = "localhost", al_sec timeout=0)
	:	Socket(port, address, timeout, true)
	{}
	
	/// Send data over a network
	
	/// @param[in] buffer	The buffer of data to send
	/// @param[in] len		The length, in bytes, of the buffer
	size_t send(const char * buffer, size_t len){ return Socket::send(buffer, len); }
};



/// Socket for receiving data over a network
class SocketRecv : public Socket {
public:

	/// @param[in] port		Port number
	/// @param[in] address	IP address. If 0, will bind all network interfaces to socket.
	/// @param[in] timeout	< 0: block forever; = 0: no blocking; > 0 block with timeout
	SocketRecv(unsigned int port, const char * address = 0, al_sec timeout=0)
	:	Socket(port, address, timeout, false)
	{}
	
	/// Read data from a network

	/// @param[in] buffer	A buffer to copy the received data into
	/// @param[in] maxlen	The maximum length, in bytes, of data to copy
	size_t recv(char * buffer, size_t maxlen){ return Socket::recv(buffer, maxlen); }
};


} // al::

#endif /* include guard */
