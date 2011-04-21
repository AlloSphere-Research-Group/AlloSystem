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
/*
Nomenclature:
Server side:
bind		Assigns a socket an address, i.e. a specified local port number and IP 
			address.
	
listen		After a socket has been associated with an address, listen 
			prepares it for incoming connections.

accept		Accepts a received incoming attempt to create a new TCP connection 
			from the remote client, and creates a new socket associated with the
			socket address pair of this connection.

Client side:
connect		Assigns a free local port number to a socket. In case of a TCP 
			socket, it causes an attempt to establish a new TCP connection.

Domain:
PF_INET			network protocol IPv4
PF_INET6		IPv6
PF_UNIX			local socket (using a file)

Type:
SOCK_STREAM		(reliable stream-oriented service or Stream Sockets)
SOCK_DGRAM		(datagram service or Datagram Sockets)
SOCK_SEQPACKET	(reliable sequenced packet service)
SOCK_RAW		(raw protocols atop the network layer)

Protocol:
IPPROTO_TCP
IPPROTO_SCTP
IPPROTO_UDP
IPPROTO_DCCP

TCP:
(PF_INET, PF_INET6), (SOCK_STREAM), (IPPROTO_TCP)
TCP Server:
1) Create a TCP socket
2) Bind socket to the listen port, with a call to bind()
3) Prepare socket to listen for connections with a call to listen()
4) Accepting incoming connections, via a call to accept()
5) Communicate with remote host, which can be done through, e.g., send()
6) Close each socket that was opened, once it is no longer needed, using close()
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
	unsigned int port() const;

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
	Socket(unsigned int port, const char * address, al_sec timeout, bool sender);

	virtual ~Socket();

	bool open(unsigned int port, const char * address, al_sec timeout, bool sender);
	size_t recv(char * buffer, size_t maxlen);
	size_t send(const char * buffer, size_t len);

private:
	class Impl; Impl * mImpl;
};



/// Socket for sending data over a network
class SocketSend : public Socket {
public:
	SocketSend(){}

	/// @param[in] port		Port number
	/// @param[in] address	IP address
	/// @param[in] timeout	< 0: block forever; = 0: no blocking; > 0 block with timeout
	SocketSend(unsigned int port, const char * address = "localhost", al_sec timeout=0)
	:	Socket(port, address, timeout, true)
	{}

	/// Open socket closing and reopening if currently open
	bool open(unsigned int port, const char * address, al_sec timeout=0){
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

	/// @param[in] port		Port number
	/// @param[in] address	IP address. If empty, will bind all network interfaces to socket.
	/// @param[in] timeout	< 0: block forever; = 0: no blocking; > 0 block with timeout
	SocketRecv(unsigned int port, const char * address = "", al_sec timeout=0)
	:	Socket(port, address, timeout, false)
	{}

	/// Open socket closing and reopening if currently open
	bool open(unsigned int port, const char * address, al_sec timeout=0){
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
