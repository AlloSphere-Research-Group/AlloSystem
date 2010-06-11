#include <cstring>
#include "io/al_Socket.hpp"
#include "system/al_Config.h"

#include "apr-1/apr_general.h"
#include "apr-1/apr_errno.h"
#include "apr-1/apr_pools.h"
#include "apr-1/apr_network_io.h"

namespace al{

static apr_status_t check_apr(apr_status_t err) {
	char errstr[1024];
	if (err != APR_SUCCESS) {
		apr_strerror(err, errstr, 1024);
		fprintf(stderr, errstr);
		//throw new osc::Exception(errstr);
	}
	return err;
}


struct Socket::Impl{

	Impl(unsigned int port, const char * address, bool sender){
		al_initialize();
		check_apr(apr_pool_create(&mPool, NULL));
	
		/* @see http://dev.ariel-networks.com/apr/apr-tutorial/html/apr-tutorial-13.html */
		// create socket:
		apr_sockaddr_t * sa;
		apr_socket_t * sock;
		check_apr(apr_sockaddr_info_get(&sa, address, APR_INET, port, 0, mPool));
		check_apr(apr_socket_create(&sock, sa->family, SOCK_DGRAM, APR_PROTO_UDP, mPool));
		
		if(sender)	check_apr(apr_socket_connect(sock, sa));
		else		check_apr(apr_socket_bind(sock, sa));
		
		check_apr(apr_socket_opt_set(sock, APR_SO_NONBLOCK, 1));
		mAddress = sa;
		mSock = sock;
	}
	
	~Impl(){
		apr_socket_close(mSock);
		apr_pool_destroy(mPool);
	}


	static apr_pool_t * defaultPool(){
		static apr_pool_t * p = 0;
		if(!p) check_apr(apr_pool_create(&p, NULL));
		return p;
	}

	apr_pool_t * mPool;
	apr_sockaddr_t * mAddress;
	apr_socket_t * mSock;
};




Socket::Socket(unsigned int port, const char * address, bool sender) 
: mImpl(new Impl(port, address, sender))
{
}

Socket::~Socket(){ delete mImpl; }

size_t Socket::recv(char * buffer, size_t maxlen) {
	apr_size_t len = maxlen;
	check_apr(apr_socket_recv(mImpl->mSock, buffer, &len));
	return len;
}

size_t Socket::send(const char * buffer, size_t len) {
	apr_size_t size = len;
	//check_apr(apr_socket_send(mSock, buffer, &size));
	apr_socket_send(mImpl->mSock, buffer, &size);
	return size;
}

std::string Socket::hostName(){
	char buf[256];
	check_apr(apr_gethostname(buf, sizeof(buf), Impl::defaultPool()));
	return buf;
}

} // al::
