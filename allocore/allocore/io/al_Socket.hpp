#ifndef INC_AL_SOCKET_HPP
#define INC_AL_SOCKET_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	Utilties for network socket sending/receiving

	Author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
	Lance Putnam, 2010, putnam.lance@gmail.com
*/

#include <string>
#include <cstdint>

namespace al{

/// @addtogroup allocore
/// @{

/// A network socket
class Socket{
public:

	/// Bit masks for specifying a transmission protocol
	enum{
		// IPv4/v6 protocols
		TCP		=   6,	/**< Transmission Control Protocol */
		UDP		=  17,	/**< User Datagram Protocol */
		SCTP	= 132,	/**< Stream Control Transmission Protocol */

		// Transmission types
		STREAM	= 1<<8,	/**< Connection-based, reliable byte stream */
		DGRAM	= 2<<8,	/**< Datagram; connectionless, unreliable fixed-length messages */

		// Families
		INET	= 1<<16, /**< IPv4 Internet protocols */
		INET6	= 2<<16  /**< IPv6 Internet protocols */
	};


	/// Create uninitialized socket
	Socket();

	/// @param[in] port		Port number (valid range is 0-65535)
	/// @param[in] address	IP address
	/// @param[in] timeout	< 0 is block forever, 0 is no blocking, > 0 is block for timeout seconds
	/// @param[in] type		Protocol type
	Socket(uint16_t port, const char * address, float timeout, int type);

	virtual ~Socket();


	/// Get name of current host
	static std::string hostName();

	/// IP address of current host
	static std::string hostIP();


	/// Returns whether socket is open
	bool opened() const;

	/// Get IP address string
	const std::string& address() const;

	/// Get port number
	uint16_t port() const;

	/// Get timeout duration, in seconds
	float timeout() const;


	/// Open socket (reopening if currently open)
	bool open(uint16_t port, const char * address, float timeout, int type);

	/// Close the socket
	void close();

	/// Bind current (local) address to socket. Called on a server socket.
	bool bind();

	/// Connect socket to current (remote) address.

	/// Called on a client socket. In case of a TCP socket, this causes an
	/// attempt to establish a new TCP connection.
	bool connect();


	/// Set socket timeout

	/// This timeout applies to send/recv, accept, and connect calls.
	///
	/// @param[in] t	Timeout in seconds.
	///					If t > 0, the socket blocks with timeout t.
	///					If t = 0, the socket never blocks.
	///					If t < 0, the socket blocks forever.
	/// Note that setting the timeout will close and re-open the socket.
	void timeout(float t);


	/// Read data from a network

	/// @param[in] buffer	A buffer to copy the received data into
	/// @param[in] maxlen	The maximum length, in bytes, of data to copy
	/// @param[out] from	The sender's IP address is copied here if pointer not null
	/// \returns bytes read or a negative value if an error occured
	//
	/// Note: to ensure receipt of all messages in the queue, use
	/// while(recv()){}
	///
	/// The from pointer should be at least the size of Message::mSenderAddr
	int recv(char * buffer, int maxlen, char *from = nullptr);

	/// Send data over a network

	/// @param[in] buffer	The buffer of data to send
	/// @param[in] len		The length, in bytes, of the buffer
	/// \returns bytes sent or a negative value if an error occured
	int send(const char * buffer, int len);


	/// Listen for incoming connections from remote clients

	/// After a socket has been associated with an address, listen prepares it
	/// for incoming connections. This is only relevent for server sockets using
	/// stream-oriented connections, such as TCP.
	bool listen();

	/// Check for an incoming socket connection

	/// Accepts a received incoming attempt to create a new TCP connection
	/// from the remote client, and creates a new socket associated with the
	/// socket address pair of this connection.
	bool accept(Socket& sock);

protected:
	// Called after a successful call to open
	virtual bool onOpen(){ return true; }

private:
	class Impl; Impl * mImpl;
};


/// Client socket

/// A client socket connects to a remote address to which it sends data to
/// (e.g., a request to a server) and possibly receives data from
/// (e.g., a response from a server).
class SocketClient : public Socket {
public:

	SocketClient(){}

	/// @param[in] port		Remote port number (valid range is 0-65535)
	/// @param[in] address	Remote IP address
	/// @param[in] timeout	< 0 is block forever, 0 is no blocking, > 0 is block for timeout seconds
	/// @param[in] type		Protocol type
	SocketClient(
		uint16_t port, const char * address = "localhost",
		float timeout = 0, int type = UDP|DGRAM
	)
	:	Socket(port, address, timeout, type)
	{
		SocketClient::onOpen();
	}

protected:
	virtual bool onOpen();
};


/// Server socket

/// A server socket typically waits and listens for incoming socket connections.
/// After accepting an incoming connection, data is received and possibly sent
/// through the accepted socket.
class SocketServer : public Socket {
public:

	SocketServer(){}

	/// @param[in] port		Local port number (valid range is 0-65535)
	/// @param[in] address	Local IP address. If empty, will bind all network interfaces to socket.
	/// @param[in] timeout	< 0 is block forever, 0 is no blocking, > 0 is block for timeout seconds
	/// @param[in] type		Protocol type
	SocketServer(
		uint16_t port, const char * address = "",
		float timeout = 0, int type = UDP|DGRAM
	)
	:	Socket(port, address, timeout, type)
	{
		SocketServer::onOpen();
	}

protected:
	virtual bool onOpen();
};


/// \deprecated Use SocketClient
typedef SocketClient SocketSend;

/// \deprecated Use SocketServer
typedef SocketServer SocketRecv;

/// @} // end allocore group

} // al::
#endif
