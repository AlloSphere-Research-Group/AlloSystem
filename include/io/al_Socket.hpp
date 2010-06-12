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

namespace al{


class Socket{
public:

	unsigned int port() const { return mPort; }

	/// Get name of current host
	static std::string hostName();
	
	/// IP address of current host
	static std::string hostIP();

protected:
	
	/// @sender: true if Socket will send
	Socket(unsigned int port, const char * address, bool sender);
	virtual ~Socket();
	
protected:
	unsigned int mPort;

	size_t recv(char * buffer, size_t maxlen);
	size_t send(const char * buffer, size_t len);

private:
	class Impl; Impl * mImpl;
};



/// Sending socket
class SocketSend : public Socket {
public:

	/// @param[in] port		Port number
	/// @param[in] address	IP address
	SocketSend(unsigned int port, const char * address = "localhost")
	:	Socket(port, address, true)
	{}
	
	/// Send data over a network
	size_t send(const char * buffer, size_t len){ return Socket::send(buffer, len); }
};



/// Receiving socket
class SocketRecv : public Socket {
public:

	/// @param[in] port		Port number
	/// @param[in] address	IP address. If 0, will bind all network interfaces to socket.
	SocketRecv(unsigned int port, const char * address = 0)
	:	Socket(port, address, false)
	{}
	
	/// Read data from a network
	size_t recv(char * buffer, size_t maxlen){ return Socket::recv(buffer, maxlen); }
};


} // al::

#endif /* include guard */
