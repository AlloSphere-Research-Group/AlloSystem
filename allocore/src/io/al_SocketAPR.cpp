/*
TCP:
(PF_INET, PF_INET6), (SOCK_STREAM), (IPPROTO_TCP)
TCP Server:
1) Create a TCP socket
2) Bind socket to the listen port, with a call to bind()
3) Prepare socket to listen for connections with a call to listen()
4) Accepting incoming connections, via a call to accept()
5) Communicate with remote host, which can be done through, e.g., send()
6) Close each socket that was opened, once it is no longer needed, using close()

From http://stackoverflow.com/questions/6189831/whats-the-purpose-of-using-sendto-recvfrom-instead-of-connect-send-recv-with-ud:

It is important to understand that TCP is "Connection Oriented" and UDP is "Connection-less" protocol:

TCP: You need to connect first prior to sending/receiving data to/from a remote host.
UDP: No connection is required. You can send/receive data to/from any host.
Therefore, you will need to use sendto() on UDP socket in order to specify the destination. Similarly, you would want to use recvfrom() to know where the UDP data was received from.

You can actually use connect() on UDP socket as an option. In that case, you can use send()/recv() on the UDP socket to send data to the address specified with the connect() and to receive data only from the address. (The connect() on UDP socket merely sets the default peer address and you can call connect() on UDP socket as many times as you want, and the connect() on UDP socket, of course, does not perform any handshake for connection.)
*/

#include <cstring>
#include "allocore/io/al_Socket.hpp"
#include "allocore/system/al_Config.h"
#include "allocore/system/al_Printing.hpp"

#include "../private/al_ImplAPR.h"
#if defined(AL_LINUX)
#include "apr-1.0/apr_network_io.h"
#else
#include "apr-1/apr_network_io.h"
#endif

#define PRINT_SOCKADDR(s)\
	printf("%s %s\n", s->hostname, s->servname);

namespace al{

struct Socket::Impl : public ImplAPR {

	Impl()
	:	mPort(0), mAddress(""), mSockAddr(0), mSock(0), mTimeout(-1), mType(0)
	{}

	Impl(uint16_t port, const char * address, al_sec timeout_, int type)
	:	mPort(port), mAddress(address), mSockAddr(0), mSock(0), mTimeout(-1), mType(0)
	{
		// opens the socket also:
		if (! open(port, address, timeout_, type)) {
			AL_WARN("Socket::Impl failed to open port %d / address \"%s\"\n", port, address);
		}
	}

	void close(){
		if(opened()){
			check_apr(apr_socket_close(mSock));
			mSock=0;
		}
	}

	#define BAILONFAIL(func)\
		if(APR_SUCCESS != check_apr(func)){\
			AL_WARN("failed to create socket at %s:%i\n", address.c_str(), port);\
			close();\
			return false;\
		}

	bool open(uint16_t port, std::string address, al_sec timeoutSec, int type){
		close();

		mPort = port;
		mAddress = address;
		mType = type;

		int sockProto = type & 127;
		switch(sockProto){
		case TCP:  sockProto = APR_PROTO_TCP; break;
		case UDP:  sockProto = APR_PROTO_UDP; break;
		case SCTP: sockProto = APR_PROTO_SCTP; break;
		default:;
		}

		int sockType = type & (127<<8);
		switch(sockType){
		case STREAM: sockType = SOCK_STREAM; break;
		case DGRAM:  sockType = SOCK_DGRAM; break;
		case 0: // unspecified; choose sensible default, if possible
			switch(sockProto){
			case TCP:  sockType = SOCK_STREAM; break;
			case UDP:
			case SCTP: sockType = SOCK_DGRAM; break;
			default:;
			}
		}

		int sockFamily = type & (127<<16);
		switch(sockFamily){
		case 0: // unspecified
		case INET:  sockFamily = APR_INET; break;
		case INET6:
		#ifdef APR_INET6
			sockFamily = APR_INET6; break;
		#else
			AL_WARN("Socket::INET6 not supported on this platform.");
			return false;
		#endif
		default:;
		}

		/*
		The APR setup procedure is as follows:
		Client:
			apr_sockaddr_info_get
			apr_socket_create

			apr_socket_opt_set		| Set timeout for connect
			apr_socket_timeout_set	|

			apr_socket_connect

			apr_socket_opt_set		| Set timeout for send/recv
			apr_socket_timeout_set	|

		Server:
			apr_sockaddr_info_get
			apr_socket_create

			apr_socket_opt_set		| Set timeout for send/recv
			apr_socket_timeout_set	|

			apr_socket_bind			| These are always non-blocking calls
			apr_socket_listen		|

		@see http://dev.ariel-networks.com/apr/apr-tutorial/html/apr-tutorial-13.html
		*/

		BAILONFAIL(
			apr_sockaddr_info_get(
				&mSockAddr,
				mAddress[0] ? mAddress.c_str() : NULL, sockFamily, mPort, 0, mPool
			)
		);

		BAILONFAIL(
			apr_socket_create(&mSock, mSockAddr->family, sockType, sockProto, mPool)
		);

		// Set timeout
		timeout(timeoutSec);

		return true;
	}

	bool reopen(){
		return open(mPort, mAddress, mTimeout, mType);
	}

	bool bind(){ // for server-side
		if(opened()){
			apr_status_t res = check_apr(apr_socket_bind(mSock, mSockAddr));
			return APR_SUCCESS == res;
		}
		return false;
	}

	bool connect(){ // for client-side
		if(opened()){
			// The timeout works differently for a blocking-with-timeout connect
			if(mTimeout > 0){
				check_apr(apr_socket_opt_set(mSock, APR_SO_NONBLOCK, 1));
				check_apr(apr_socket_timeout_set(mSock, apr_interval_time_t(mTimeout * 1.0e6)));
			}

			apr_status_t res = check_apr(apr_socket_connect(mSock, mSockAddr));

			// Set timeout back to what it was if blocking-with-timeout
			if(mTimeout > 0){
				timeout(mTimeout);
			}

			return APR_SUCCESS == res;
		}
		return false;
	}

	void timeout(al_sec v){
		mTimeout = v;

		// Note: these timeouts apply only for send/recv and accept
		// Non-blocking:
		if(mTimeout == 0){
			check_apr(apr_socket_opt_set(mSock, APR_SO_NONBLOCK, 1));
			check_apr(apr_socket_timeout_set(mSock, 0));
		}

		// Blocking-with-timeout:
		else if(mTimeout > 0){
			#ifdef AL_WINDOWS
				check_apr(apr_socket_opt_set(mSock, APR_SO_NONBLOCK, 0));
			#else
				check_apr(apr_socket_opt_set(mSock, APR_SO_NONBLOCK, 1));
			#endif
			check_apr(apr_socket_timeout_set(mSock, apr_interval_time_t(mTimeout * 1.0e6)));
		}

		// Blocking-forever:
		else{
			check_apr(apr_socket_opt_set(mSock, APR_SO_NONBLOCK, 0));
			check_apr(apr_socket_timeout_set(mSock, -1));
		}
	}

	bool listen(){
		/* APR_SO_REUSEADDR is useful for a socket listening process.
		It specifies that the "The rules used in validating addresses supplied
		to bind should allow reuse of local addresses." */
		apr_socket_opt_set(mSock, APR_SO_REUSEADDR, 1);
		apr_status_t res = apr_socket_listen(mSock, SOMAXCONN);
		if(APR_SUCCESS != res){
			//AL_WARN("Failed to listen on socket");
			return false;
		}
		return true;
	}

	bool accept(Socket::Impl * newSock){
		newSock->close();
		// TODO: timeout for accept
		apr_status_t res = apr_socket_accept(&newSock->mSock, mSock, mPool);
		if(APR_SUCCESS != res){
			//AL_WARN("Failed to accept socket");
			return false;
		}
		// Inherit timeout from parent
		newSock->timeout(mTimeout);
		return true;
	}

	bool opened() const { return 0!=mSock; }

	uint16_t mPort;
	std::string mAddress;
	apr_sockaddr_t * mSockAddr;
	apr_socket_t * mSock;
	apr_sockaddr_t mFromAddress;
	al_sec mTimeout;
	int mType;
};



Socket::Socket()
:	mImpl(new Impl)
{}

Socket::Socket(uint16_t port, const char * address, al_sec timeout, int type)
: mImpl(0)
{
	mImpl = new Impl(port, address, timeout, type);
	if (mImpl == 0) {
		AL_WARN("Socket::Socket(uint16_t, const char *, al_sec, bool): mImpl is zero!!!\n");
	}

}

Socket::~Socket(){
	close();
	delete mImpl;
}

const std::string& Socket::address() const { return mImpl->mAddress; }

bool Socket::opened() const { return mImpl->opened(); }

uint16_t Socket::port() const { return mImpl->mPort; }

al_sec Socket::timeout() const { return mImpl->mTimeout; }

bool Socket::bind(){ return mImpl->bind(); }

bool Socket::connect(){ return mImpl->connect(); }

void Socket::close(){ mImpl->close(); }

bool Socket::open(uint16_t port, const char * address, al_sec timeout, int type){
	return
		mImpl->open(port, address, timeout, type)
		&& onOpen();
}

void Socket::timeout(al_sec v){
	mImpl->timeout(v);
}


size_t Socket::recv(char * buffer, size_t maxlen, char *from) {
	apr_size_t len = maxlen;

	// printf("About to call apr_socket_recv(%p, %p, %d)\n", mImpl->mSock, buffer, len);
 	apr_int32_t flags = 0;
	apr_status_t r = apr_socket_recvfrom(&mImpl->mFromAddress, mImpl->mSock, flags, buffer, &len);
	char addr[32] = "";
	apr_status_t ret = apr_sockaddr_ip_getbuf(addr, 32, &mImpl->mFromAddress);
	if (len > 0 && from != nullptr && ret == APR_SUCCESS) {
		strncpy(from, addr, 15);
	}
	// printf("apr_socket_recv returned\n");

	// only error check if not error# 35: Resource temporarily unavailable
	if(len){ check_apr(r); }
	return len;
}

size_t Socket::send(const char * buffer, size_t len) {
	apr_size_t size = len;
	if (mImpl->opened()) {
		//check_apr(apr_socket_send(mSock, buffer, &size));
		apr_socket_send(mImpl->mSock, buffer, &size);
	} else {
		size = 0;
	}
	return size;
}


std::string Socket::hostIP(){
	ImplAPR apr;
	char * addr;
	apr_sockaddr_t * sa;
	check_apr(apr_sockaddr_info_get(&sa, hostName().c_str(), APR_INET, 8000, 0, apr.pool()));
	//printf("%p sa\n", sa);
	while(sa != 0) {
		check_apr(apr_sockaddr_ip_get(&addr, sa));
		//printf("%s %s %s %d %d\n", addr, sa->hostname, sa->servname, sa->port, sa->family);
		sa = sa->next;
	}
	return addr;
}

std::string Socket::hostName(){
	char buf[APRMAXHOSTLEN+1];
	ImplAPR apr;
	check_apr(apr_gethostname(buf, sizeof(buf), apr.pool()));
	//printf("host %s\n", buf);
	return buf;
}


bool Socket::listen(){
	return mImpl->listen();
}

bool Socket::accept(Socket& sock){
	return mImpl->accept(sock.mImpl);
}


bool SocketClient::onOpen(){
	return connect();
}


bool SocketServer::onOpen(){
	return bind();
}

} // al::
