#include <cstring>
#include "allocore/io/al_Socket.hpp"
#include "allocore/system/al_Config.h"

#include "../private/al_ImplAPR.h"
#ifdef AL_LINUX
#include "apr-1.0/apr_network_io.h"
#else
#include "apr-1/apr_network_io.h"
#endif

#define PRINT_SOCKADDR(s)\
	printf("%s %s\n", s->hostname, s->servname);

namespace al{

struct Socket::Impl : public ImplAPR {

	Impl(unsigned int port, const char * address, al_sec timeout_, bool sender)
	:	ImplAPR(), mPort(port), mAddressString(address ? address : ""), mAddress(0), mSock(0), mSender(sender)
	{
		// opens the socket also:
		timeout(timeout_);
	}

	void close(){
		if(opened()){
			check_apr(apr_socket_close(mSock));
			mSock=0;
		}
	}

	void open(unsigned int port, std::string address, bool sender){

		close();

		// TODO: check_apr results should jump to an err label and return an uninitialized socket!

		/* @see http://dev.ariel-networks.com/apr/apr-tutorial/html/apr-tutorial-13.html */

		mPort = port;
		mSender = sender;
		mAddressString = address;
		if (APR_SUCCESS == check_apr(apr_sockaddr_info_get(&mAddress, (mAddressString[0]) ? mAddressString.c_str() : 0, APR_INET, mPort, 0, mPool))) {
			check_apr(apr_socket_create(&mSock, mAddress->family, SOCK_DGRAM, APR_PROTO_UDP, mPool));

			if(mSock){
				// Assign address to socket. If TCP, establish new connection.
				if(mSender)	check_apr(apr_socket_connect(mSock, mAddress));
				else		check_apr(apr_socket_bind(mSock, mAddress));
			}
		} else {
			printf("failed to create socket at %s:%i\n", address.c_str(), port);
		}
	}
	
	// note that setting timeout will close and re-open the socket:
	void timeout(al_sec v){
		open(mPort, mAddressString, mSender);
		
		if(v == 0){
			// non-blocking:			APR_SO_NONBLOCK==1(on),  then timeout=0
			check_apr(apr_socket_opt_set(mSock, APR_SO_NONBLOCK, 1));
			check_apr(apr_socket_timeout_set(mSock, 0));
		}
		else if(v > 0){
			// blocking-with-timeout:	APR_SO_NONBLOCK==0(off), then timeout>0
			check_apr(apr_socket_opt_set(mSock, APR_SO_NONBLOCK, 0));
			check_apr(apr_socket_timeout_set(mSock, (apr_interval_time_t)(v * 1.0e6)));

		}
		else{
			// blocking-forever:		APR_SO_NONBLOCK==0(off), then timeout<0
			check_apr(apr_socket_opt_set(mSock, APR_SO_NONBLOCK, 0));
			check_apr(apr_socket_timeout_set(mSock, -1));
		}
		mTimeout = v;
	}
	
	bool opened(){ return 0!=mSock; }

	unsigned int mPort;
	std::string mAddressString;
	apr_sockaddr_t * mAddress;
	apr_socket_t * mSock;
	al_sec mTimeout;
	bool mSender;
};




Socket::Socket(unsigned int port, const char * address, al_sec timeout, bool sender)
: mImpl(0)
{
	mImpl = new Impl(port, address, timeout, sender);
}

Socket::~Socket(){
	close();
	delete mImpl;
}

void Socket::close(){ mImpl->close(); }

void Socket::open(unsigned int port, const char * address, bool sender){
	mImpl->open(port, address, sender);
}

unsigned int Socket::port() const { return mImpl->mPort; }

void Socket::timeout(al_sec v){ mImpl->timeout(v); }

al_sec Socket::timeout() const {
	return mImpl->mTimeout;
}

size_t Socket::recv(char * buffer, size_t maxlen) {
	apr_size_t len = maxlen;
	apr_status_t r = apr_socket_recv(mImpl->mSock, buffer, &len);
	
	// only error check if not error# 35: Resource temporarily unavailable
	if(len){ check_apr(r); }
	return len;
}

size_t Socket::send(const char * buffer, size_t len) {
	apr_size_t size = len;
	//check_apr(apr_socket_send(mSock, buffer, &size));
	apr_socket_send(mImpl->mSock, buffer, &size);
	return size;
}


std::string Socket::hostIP(){
	ImplAPR apr;
	char * addr;
	apr_sockaddr_t * sa;
	apr_sockaddr_info_get(&sa, hostName().c_str(), APR_INET, 8000, 0, apr.pool());
	while(sa) {
		apr_sockaddr_ip_get(&addr, sa);
		//printf("%s %s %s %d %d\n", addr, sa->hostname, sa->servname, sa->port, sa->family);
		sa = sa->next;
	}
	return addr;
}

std::string Socket::hostName(){
	char buf[APRMAXHOSTLEN+1];
	ImplAPR apr;
	check_apr(apr_gethostname(buf, sizeof(buf), apr.pool()));
	return buf;
}

} // al::
