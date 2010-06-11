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

/// Get name of current host
//std::string getHostName();


class Socket{
public:

	Socket(unsigned int port, const char * address, bool sender);
	virtual ~Socket();

	unsigned int port() const { return mPort; }
	
protected:
	unsigned int mPort;

	size_t recv(char * buffer, size_t maxlen);
	size_t send(const char * buffer, size_t len);

private:
	class Impl; Impl * mImpl;
};


class SocketSend : public Socket {
public:
	SocketSend(const char * address, unsigned int port)
	:	Socket(port, address, true)
	{}
	
	size_t send(const char * buffer, size_t len){ return Socket::send(buffer, len); }
};



class SocketRecv : public Socket {
public:
	SocketRecv(unsigned int port)
	:	Socket(port, NULL, false)
	{}
	
	size_t recv(char * buffer, size_t maxlen){ return Socket::recv(buffer, maxlen); }
};


} // al::

#endif /* include guard */
