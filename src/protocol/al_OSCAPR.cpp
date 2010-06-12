#include "protocol/al_OSCAPR.hpp"

#include "../private/al_ImplAPR.h"

namespace osc {

//static apr_status_t check_apr(apr_status_t err) {
//	char errstr[1024];
//	if (err != APR_SUCCESS) {
//		apr_strerror(err, errstr, 1024);
//		fprintf(stderr, errstr);
//		throw new osc::Exception(errstr);
//	}
//	return err;
//}

static void osc_parsemessage(const osc::ReceivedMessage & p) {
	printf("address %s tags %s args %ld\n", p.AddressPattern(), p.TypeTags(), p.ArgumentCount());
	// etc.
}

static void osc_parsebundle(const osc::ReceivedBundle & p, Recv::MessageParser handler, void * userdata) {
	for(osc::ReceivedBundle::const_iterator i=p.ElementsBegin(); i != p.ElementsEnd(); ++i) {
		if(i->IsBundle()) {
			osc_parsebundle(osc::ReceivedBundle(*i), handler, userdata);
		} else {
			(handler)(osc::ReceivedMessage(*i), userdata);
		}
	}
}

using namespace al;

Recv::Recv(unsigned int port) {
	
	check_apr(apr_pool_create(&mPool, NULL));
	
	/* @see http://dev.ariel-networks.com/apr/apr-tutorial/html/apr-tutorial-13.html */
	// create socket:
	apr_sockaddr_t * sa;
	apr_socket_t * sock;
	check_apr(apr_sockaddr_info_get(&sa, NULL, APR_INET, port, 0, mPool));
	check_apr(apr_socket_create(&sock, sa->family, SOCK_DGRAM, APR_PROTO_UDP, mPool));
	check_apr(apr_socket_bind(sock, sa));
	check_apr(apr_socket_opt_set(sock, APR_SO_NONBLOCK, 1));
	mAddress = sa;
	mSock = sock;
}

Recv::~Recv() {
	apr_socket_close(mSock);
	apr_pool_destroy(mPool);
}

size_t Recv::recv(char * buffer, size_t maxlen) {
	apr_size_t len = maxlen;
	check_apr(apr_socket_recv(mSock, buffer, &len));
	return len;
}

size_t Recv::recv(MessageParser handler, void * userdata, size_t maxlen) {
	char buffer[maxlen];
	apr_size_t len = maxlen;
	apr_status_t res = apr_socket_recv(mSock, buffer, &len);
	if (res == 0 && len) {
		osc::ReceivedPacket p(buffer, len);
		if(p.IsBundle()) {
			osc_parsebundle(osc::ReceivedBundle(p), handler, userdata);
		} else {
			(handler)(osc::ReceivedMessage(p), userdata);
		}
	}
	return len;
}
	
Send::Send(const char * address, unsigned int port) 
: mPort(port) {

	check_apr(apr_pool_create(&mPool, NULL));

	/* @see http://dev.ariel-networks.com/apr/apr-tutorial/html/apr-tutorial-13.html */
	// create socket:
	apr_sockaddr_t * sa;
	apr_socket_t * sock;
	check_apr(apr_sockaddr_info_get(&sa, address, APR_INET, port, 0, mPool));
	check_apr(apr_socket_create(&sock, sa->family, SOCK_DGRAM, APR_PROTO_UDP, mPool));
	check_apr(apr_socket_connect(sock, sa));
	check_apr(apr_socket_opt_set(sock, APR_SO_NONBLOCK, 1));
	mAddress = sa;
	mSock = sock;
}

Send::~Send() {
	apr_socket_close(mSock);
	apr_pool_destroy(mPool);
}

size_t Send::send(const char * buffer, size_t len) {
	apr_size_t size = len;
	//check_apr(apr_socket_send(mSock, buffer, &size));
	apr_socket_send(mSock, buffer, &size);
	return size;
}

size_t Send::send(const osc::OutboundPacketStream & packet) {
	apr_size_t size = packet.Size();
	//check_apr(apr_socket_send(mSock, packet.Data(), &size));
	apr_socket_send(mSock, packet.Data(), &size);
	return size;
}
	
} // namespace
