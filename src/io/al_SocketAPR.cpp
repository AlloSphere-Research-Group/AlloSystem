#include <cstring>
#include "io/al_Socket.hpp"
#include "system/al_Config.h"

#include "../private/al_ImplAPR.h"
#include "apr-1/apr_network_io.h"

#define PRINT_SOCKADDR(s)\
	printf("%s %s\n", s->hostname, s->servname);

namespace al{

struct Socket::Impl : public ImplAPR {

	Impl(unsigned int port, const char * address, bool sender) : ImplAPR() {
		
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

//		char * buf;		
////		apr_sockaddr_ip_get(&buf, mAddress);
//		apr_getnameinfo(&buf, sa, 0);	
////		char * scopeid;
////		apr_port_t p = port;
////		apr_parse_addr_port(&buf, &scopeid, &p, "localhost", defaultPool());
//		printf("%s\n", buf);

	}

	apr_sockaddr_t * mAddress;
	apr_socket_t * mSock;
};




Socket::Socket(unsigned int port, const char * address, bool sender) 
: mImpl(new Impl(port, address, sender))
{
}

Socket::~Socket(){
	close();
	delete mImpl;
}

void Socket::close(){
	apr_socket_close(mImpl->mSock);
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
