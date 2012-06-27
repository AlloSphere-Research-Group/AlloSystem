#ifndef INCLUDE_AL_IO_SOCKET_HPP
#define INCLUDE_AL_IO_SOCKET_HPP 1

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
	Utilties for network socket sending/receiving

	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
	Lance Putnam, 2010, putnam.lance@gmail.com
*/


#include <string>
#include "allocore/system/al_Config.h"

namespace al{


/// Base class for network socket
class Socket{
public:

	/// Returns whether socket is open
	bool opened() const;

	/// Get IP address string
	const std::string& address() const;

	/// Get port number
	uint16_t port() const;

	/// Get timeout duration, in seconds
	al_sec timeout() const;


	/// Close the socket
	void close();

	/// Set timeout. < 0: block forever; = 0: no blocking; > 0 block with timeout

	/// Note that setting timeout will close and re-open the socket.
	///
	void timeout(al_sec v);


	/// Get name of current host
	static std::string hostName();
	
	/// IP address of current host
	static std::string hostIP();

protected:
	Socket();

	/// @sender: true if Socket will send
	Socket(uint16_t port, const char * address, al_sec timeout, bool sender);

	virtual ~Socket();

	bool open(uint16_t port, const char * address, al_sec timeout, bool sender);
	size_t recv(char * buffer, size_t maxlen);
	size_t send(const char * buffer, size_t len);

private:
	class Impl; Impl * mImpl;
};



/// Socket for sending data over a network
class SocketSend : public Socket {
public:
	SocketSend(){}

	/// @param[in] port		Port number (valid range is 0-65535)
	/// @param[in] address	IP address
	/// @param[in] timeout	< 0: block forever; = 0: no blocking; > 0 block with timeout
	SocketSend(uint16_t port, const char * address = "localhost", al_sec timeout=0)
	:	Socket(port, address, timeout, true)
	{}

	/// Open socket closing and reopening if currently open
	bool open(uint16_t port, const char * address, al_sec timeout=0){
		return Socket::open(port, address, timeout, true);
	}

	/// Send data over a network
	
	/// @param[in] buffer	The buffer of data to send
	/// @param[in] len		The length, in bytes, of the buffer
	size_t send(const char * buffer, size_t len){ return Socket::send(buffer, len); }
};

/// Socket for receiving data over a network
class SocketRecv : public Socket {
public:
	SocketRecv(){}

	/// @param[in] port		Port number (valid range is 0-65535)
	/// @param[in] address	IP address. If empty, will bind all network interfaces to socket.
	/// @param[in] timeout	< 0: block forever; = 0: no blocking; > 0 block with timeout
	SocketRecv(uint16_t port, const char * address = "", al_sec timeout=0)
	:	Socket(port, address, timeout, false)
	{}

	/// Open socket closing and reopening if currently open
	bool open(uint16_t port, const char * address = "", al_sec timeout=0){
		return Socket::open(port, address, timeout, false);
	}

	/// Read data from a network

	/// @param[in] buffer	A buffer to copy the received data into
	/// @param[in] maxlen	The maximum length, in bytes, of data to copy
	/// \returns bytes read
	//
	/// Note: to ensure receipt of all messages in the queue, use 
	/// while(recv()){}
	size_t recv(char * buffer, size_t maxlen){ return Socket::recv(buffer, maxlen); }
};


} // al::

#endif /* include guard */
